#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    Display *dpy;
    Window root;
    XRRScreenResources *sr;
    XRROutputInfo *output_info;
    XRRCrtcInfo *crtc_info;
    int screen;

    // Open display
    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    // Get screen resources
    sr = XRRGetScreenResources(dpy, root);
    if (!sr) {
        fprintf(stderr, "Cannot get screen resources\n");
        XCloseDisplay(dpy);
        return 1;
    }

    printf("Screen %d: current %d x %d\n",
           screen,
           DisplayWidth(dpy, screen),
           DisplayHeight(dpy, screen));

    // Iterate through outputs
    for (int i = 0; i < sr->noutput; i++) {
        output_info = XRRGetOutputInfo(dpy, sr, sr->outputs[i]);

        printf("%s ", output_info->name);

        if (output_info->connection == RR_Connected) {
            printf("connected");

            if (output_info->crtc) {
                crtc_info = XRRGetCrtcInfo(dpy, sr, output_info->crtc);
                printf(" %dx%d+%d+%d",
                       crtc_info->width, crtc_info->height,
                       crtc_info->x, crtc_info->y);
                XRRFreeCrtcInfo(crtc_info);
            }

            // Print available modes
            printf("\n");
            for (int j = 0; j < output_info->nmode; j++) {
                XRRModeInfo *mode = NULL;
                for (int k = 0; k < sr->nmode; k++) {
                    if (sr->modes[k].id == output_info->modes[j]) {
                        mode = &sr->modes[k];
                        break;
                    }
                }
                if (mode) {
                    double refresh = 0;
                    if (mode->hTotal && mode->vTotal) {
                        refresh = (double)mode->dotClock /
                                 ((double)mode->hTotal * (double)mode->vTotal);
                    }
                    printf("   %dx%d  %.2f Hz",
                           mode->width, mode->height, refresh);
                    if (j == 0) printf(" (preferred)");
                    printf("\n");
                }
            }

        } else if (output_info->connection == RR_Disconnected) {
            printf("disconnected\n");
        } else {
            printf("unknown connection\n");
        }

        XRRFreeOutputInfo(output_info);
    }

    XRRFreeScreenResources(sr);
    XCloseDisplay(dpy);
    return 0;
}
