// Zephyrus G14 (2021) N-Key Rollover Fix for Linux v1.0
// © 2026 Dunspixel

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <linux/input.h>
#include <libevdev/libevdev-uinput.h>

int phys_fd;
int virt_fd;
struct libevdev* phys_device;
struct libevdev_uinput* virt_device;

void exit_program();

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("Please specify an input device as an argument, e.g. /dev/input/event1\n");
		return EXIT_FAILURE;
	}

	phys_fd = open(argv[1], O_RDWR);
	virt_fd = open("/dev/uinput", O_RDWR);

	struct input_event event;
	time_t prev_sec;
	suseconds_t prev_usec;

	// Initialise the virtual device and grab the existing one to override it
	libevdev_new_from_fd(phys_fd, &phys_device);
	libevdev_uinput_create_from_device(phys_device, virt_fd, &virt_device);
	ioctl(phys_fd, EVIOCGRAB, EV_KEY);
	signal(SIGINT, exit_program);

	printf("Virtual keyboard initialised. Suppressing incorrect key releases...\n");

	while (1)
	{
		read(phys_fd, &event, sizeof(event));

		// Capture all key release events
		if (event.type == EV_KEY && event.value == 0)
		{
			// If the key release event occurs on the exact same microsecond as the previous release event,
			// suppress it and inject an equivalent key press back into the same key.
			// This allows the correct release event to be triggered when the key is physically released.
			if (event.time.tv_sec == prev_sec && event.time.tv_usec == prev_usec)
			{
				printf("Suppressing incorrect release of key %d at %d.%06d\n", event.code, event.time.tv_sec, event.time.tv_usec);
				event.value = 1;
				write(phys_fd, &event, sizeof(event));
				continue;
			}

			// Track the event timestamp
			prev_sec = event.time.tv_sec;
			prev_usec = event.time.tv_usec;
		}

		// If the event does not need to be suppressed, write it to the virtual device so it functions normally
		libevdev_uinput_write_event(virt_device, event.type, event.code, event.value);
		libevdev_uinput_write_event(virt_device, EV_SYN, SYN_REPORT, 0);
	}

	return EXIT_SUCCESS;
}

void exit_program()
{
	printf("\nExiting...\n");
	libevdev_free(phys_device);
	libevdev_uinput_destroy(virt_device);
	close(phys_fd);
	close(virt_fd);
	exit(EXIT_SUCCESS);
}
