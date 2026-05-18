#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define VIDEO_DEV "/dev/video0"

#define WIDTH 1280u
#define HEIGHT 720u
#define BYTES_PER_PIXEL 2u
#define STRIDE (WIDTH * BYTES_PER_PIXEL)
#define FRAME_SIZE (STRIDE * HEIGHT)
#define NUM_BUFS 4u

#define HDMI_FB_PHYS 0x3F000000u
#define HDMI_FB_SIZE (FRAME_SIZE * NUM_BUFS)

#define UIO_MAP_SIZE 0x10000u

#define FRMBUF_AP_CTRL 0x00u
#define FRMBUF_WIDTH 0x10u
#define FRMBUF_HEIGHT 0x18u
#define FRMBUF_STRIDE 0x20u
#define FRMBUF_FORMAT 0x28u
#define FRMBUF_ADDR0 0x30u

#define FRMBUF_FORMAT_YUYV8 12u

#define VTC_CONTROL 0x000u
#define VTC_SW_ENABLE 0x00000001u
#define VTC_REG_UPDATE 0x00000002u
#define VTC_GEN_ENABLE 0x00000010u

struct video_buffer {
	void *start;
	size_t length;
	uint32_t phys;
	int queued;
};

static volatile sig_atomic_t stop_requested;

static void handle_signal(int signal_number)
{
	(void)signal_number;
	stop_requested = 1;
}

static int xioctl(int fd, unsigned long request, void *arg)
{
	int ret;

	do {
		ret = ioctl(fd, request, arg);
	} while (ret < 0 && errno == EINTR);

	return ret;
}

static void write32(volatile void *base, uint32_t offset, uint32_t value)
{
	volatile uint32_t *addr = (volatile uint32_t *)((volatile uint8_t *)base + offset);
	*addr = value;
}

static void *map_phys(uint32_t phys, size_t size)
{
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "open(/dev/mem): %s\n", strerror(errno));
		return MAP_FAILED;
	}

	void *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys);
	if (map == MAP_FAILED)
		fprintf(stderr, "mmap(/dev/mem @ 0x%08x): %s\n", phys, strerror(errno));

	close(fd);
	return map;
}

static int read_text_file(const char *path, char *buf, size_t size)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;

	ssize_t len = read(fd, buf, size - 1);
	close(fd);
	if (len < 0)
		return -1;

	buf[len] = '\0';
	while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
		buf[--len] = '\0';

	return 0;
}

static int find_uio_by_name(const char *name, char *path, size_t path_size)
{
	DIR *dir = opendir("/sys/class/uio");
	if (!dir) {
		fprintf(stderr, "opendir(/sys/class/uio): %s\n", strerror(errno));
		return -1;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, "uio", 3) != 0)
			continue;

		char name_path[512];
		char found_name[64];
		snprintf(name_path, sizeof(name_path), "/sys/class/uio/%s/name", entry->d_name);

		if (read_text_file(name_path, found_name, sizeof(found_name)) == 0 &&
		    strcmp(found_name, name) == 0) {
			snprintf(path, path_size, "/dev/%s", entry->d_name);
			closedir(dir);
			return 0;
		}
	}

	closedir(dir);
	fprintf(stderr, "UIO device named '%s' was not found\n", name);
	return -1;
}

static void *map_uio_by_name(const char *name, size_t size)
{
	char path[512];

	if (find_uio_by_name(name, path, sizeof(path)) < 0)
		return MAP_FAILED;

	int fd = open(path, O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "open(%s): %s\n", path, strerror(errno));
		return MAP_FAILED;
	}

	void *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
		fprintf(stderr, "mmap(%s): %s\n", path, strerror(errno));

	close(fd);
	return map;
}

static void setup_frmbuf_rd(void *frmbuf_rd, uint32_t phys)
{
	write32(frmbuf_rd, FRMBUF_AP_CTRL, 0x00);
	write32(frmbuf_rd, FRMBUF_WIDTH, WIDTH);
	write32(frmbuf_rd, FRMBUF_HEIGHT, HEIGHT);
	write32(frmbuf_rd, FRMBUF_STRIDE, STRIDE);
	write32(frmbuf_rd, FRMBUF_FORMAT, FRMBUF_FORMAT_YUYV8);
	write32(frmbuf_rd, FRMBUF_ADDR0, phys);
	write32(frmbuf_rd, FRMBUF_AP_CTRL, 0x81);
}

static int queue_buffer(int fd, struct video_buffer *buffers, uint32_t index)
{
	struct v4l2_buffer buf;
	struct v4l2_plane planes[1];

	memset(&buf, 0, sizeof(buf));
	memset(planes, 0, sizeof(planes));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	buf.memory = V4L2_MEMORY_USERPTR;
	buf.index = index;
	buf.length = 1;
	buf.m.planes = planes;
	planes[0].m.userptr = (unsigned long)buffers[index].start;
	planes[0].length = buffers[index].length;

	if (xioctl(fd, VIDIOC_QBUF, &buf) < 0) {
		fprintf(stderr, "VIDIOC_QBUF[%u]: %s\n", index, strerror(errno));
		return -1;
	}

	buffers[index].queued = 1;
	return 0;
}

static int setup_video(int fd, struct video_buffer *buffers, void *fb)
{
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;

	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	fmt.fmt.pix_mp.width = WIDTH;
	fmt.fmt.pix_mp.height = HEIGHT;
	fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
	fmt.fmt.pix_mp.num_planes = 1;

	if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
		fprintf(stderr, "VIDIOC_S_FMT: %s\n", strerror(errno));
		return -1;
	}

	if (fmt.fmt.pix_mp.width != WIDTH || fmt.fmt.pix_mp.height != HEIGHT) {
		fprintf(stderr, "camera selected %ux%u, expected %ux%u\n",
			fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height, WIDTH, HEIGHT);
		return -1;
	}

	if (fmt.fmt.pix_mp.pixelformat != V4L2_PIX_FMT_YUYV) {
		fprintf(stderr, "camera selected pixel format 0x%08x, expected YUYV\n",
			fmt.fmt.pix_mp.pixelformat);
		return -1;
	}

	memset(&req, 0, sizeof(req));
	req.count = NUM_BUFS;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (xioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
		fprintf(stderr, "VIDIOC_REQBUFS(USERPTR): %s\n", strerror(errno));
		return -1;
	}

	if (req.count < NUM_BUFS) {
		fprintf(stderr, "driver allocated only %u buffers\n", req.count);
		return -1;
	}

	for (uint32_t i = 0; i < NUM_BUFS; i++) {
		buffers[i].start = (uint8_t *)fb + (i * FRAME_SIZE);
		buffers[i].length = FRAME_SIZE;
		buffers[i].phys = HDMI_FB_PHYS + (i * FRAME_SIZE);
		buffers[i].queued = 0;

		memset(buffers[i].start, 0x10, buffers[i].length);

		if (queue_buffer(fd, buffers, i) < 0)
			return -1;
	}

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if (xioctl(fd, VIDIOC_STREAMON, &type) < 0) {
		fprintf(stderr, "VIDIOC_STREAMON: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

static int wait_for_frame(int fd)
{
	fd_set fds;
	struct timeval timeout;
	int ret;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	do {
		ret = select(fd + 1, &fds, NULL, NULL, &timeout);
	} while (ret < 0 && errno == EINTR);

	return ret;
}

int main(void)
{
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	void *fb = map_phys(HDMI_FB_PHYS, HDMI_FB_SIZE);
	if (fb == MAP_FAILED)
		return 1;

	void *frmbuf_rd = map_uio_by_name("v_frmbuf_rd", UIO_MAP_SIZE);
	if (frmbuf_rd == MAP_FAILED)
		return 1;

	void *vtc = map_uio_by_name("v_tc", UIO_MAP_SIZE);
	if (vtc == MAP_FAILED)
		return 1;

	write32(vtc, VTC_CONTROL, VTC_SW_ENABLE | VTC_REG_UPDATE | VTC_GEN_ENABLE);
	setup_frmbuf_rd(frmbuf_rd, HDMI_FB_PHYS);

	int video_fd = open(VIDEO_DEV, O_RDWR);
	if (video_fd < 0) {
		fprintf(stderr, "open(%s): %s\n", VIDEO_DEV, strerror(errno));
		return 1;
	}

	struct video_buffer buffers[NUM_BUFS] = {0};
	if (setup_video(video_fd, buffers, fb) < 0)
		return 1;

	printf("PCAM HDMI started: %ux%u YUYV USERPTR @ 0x%08x\n",
	       WIDTH, HEIGHT, HDMI_FB_PHYS);

	uint32_t frame_count = 0;
	time_t last_report = time(NULL);
	uint32_t timeout_count = 0;
	int displayed_index = -1;

	while (!stop_requested) {
		struct v4l2_buffer buf;
		struct v4l2_plane planes[1];

		int ready = wait_for_frame(video_fd);
		if (ready < 0) {
			fprintf(stderr, "select(%s): %s\n", VIDEO_DEV, strerror(errno));
			break;
		}
		if (ready == 0) {
			timeout_count++;
			printf("waiting for frame... (%u)\n", timeout_count);
			continue;
		}

		memset(&buf, 0, sizeof(buf));
		memset(planes, 0, sizeof(planes));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = V4L2_MEMORY_USERPTR;
		buf.length = 1;
		buf.m.planes = planes;

		if (xioctl(video_fd, VIDIOC_DQBUF, &buf) < 0) {
			if (errno == EINTR)
				continue;
			fprintf(stderr, "VIDIOC_DQBUF: %s\n", strerror(errno));
			break;
		}

		buffers[buf.index].queued = 0;
		setup_frmbuf_rd(frmbuf_rd, buffers[buf.index].phys);

		if (displayed_index >= 0 && queue_buffer(video_fd, buffers, displayed_index) < 0)
			break;
		displayed_index = (int)buf.index;

		frame_count++;
		time_t now = time(NULL);
		if (now != last_report) {
			printf("fps: %u\n", frame_count);
			frame_count = 0;
			last_report = now;
		}
	}

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	xioctl(video_fd, VIDIOC_STREAMOFF, &type);

	for (uint32_t i = 0; i < NUM_BUFS; i++) {
		buffers[i].start = NULL;
	}

	close(video_fd);
	write32(frmbuf_rd, FRMBUF_AP_CTRL, 0x00);
	write32(vtc, VTC_CONTROL, 0x00);
	munmap(frmbuf_rd, UIO_MAP_SIZE);
	munmap(vtc, UIO_MAP_SIZE);
	munmap(fb, HDMI_FB_SIZE);

	return 0;
}
