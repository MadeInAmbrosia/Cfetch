#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>

#define INFO_LINES 20
#define ASCII_LINES 8
#define C_CYAN   "\033[1;36m"
#define C_RESET  "\033[0m"
#define C_BOLD   "\033[1m"

void read_sys_file(const char *path, char *buffer, size_t size) {
    FILE *fp = fopen(path, "r");
    if (fp) {
        if (fgets(buffer, size, fp)) buffer[strcspn(buffer, "\n")] = 0;
        fclose(fp);
    } else { strncpy(buffer, "N/A", size); }
}

void get_display_server(char *server, size_t size) {
    if (getenv("WAYLAND_DISPLAY")) strncpy(server, "Wayland", size);
    else if (getenv("DISPLAY")) {
        if (access("/usr/bin/XLibre", F_OK) == 0) strncpy(server, "XLibre (X11)", size);
        else strncpy(server, "Xorg (X11)", size);
    } else {
        if (access("/tmp/.X11-unix/X0", F_OK) == 0) strncpy(server, "X11 (Socket)", size);
        else strncat(server, "TTY/None", size);
    }
}

void get_de_wm(char *de, size_t size) {
    char *xdg_de = getenv("XDG_CURRENT_DESKTOP");
    char *xdg_sess = getenv("DESKTOP_SESSION");
    if (xdg_de) strncpy(de, xdg_de, size);
    else if (xdg_sess) strncpy(de, xdg_sess, size);
    else {
        if (access("/usr/bin/dwm", F_OK) == 0 && getenv("DISPLAY")) strncpy(de, "dwm", size);
        else if (access("/usr/bin/i3", F_OK) == 0) strncpy(de, "i3", size);
        else if (access("/usr/bin/xfce4-session", F_OK) == 0) strncpy(de, "XFCE4", size);
        else if (access("/usr/bin/lxqt-session", F_OK) == 0) strncpy(de, "LXQt", size);
        else if (access("/usr/bin/openbox", F_OK) == 0) strncpy(de, "Openbox", size);
        else strncpy(de, "Unknown", size);
    }
}

void get_init_system(char *init, size_t size) {
    char comm[64] = "";
    read_sys_file("/proc/1/comm", comm, sizeof(comm));
    if (strstr(comm, "systemd")) strncpy(init, "systemd", size);
    else if (access("/run/openrc", F_OK) == 0) strncpy(init, "OpenRC", size);
    else if (access("/run/runit", F_OK) == 0) strncpy(init, "Runit", size);
    else if (access("/run/dinit", F_OK) == 0 || strstr(comm, "dinit")) strncpy(init, "dinit", size);
    else if (access("/run/s6", F_OK) == 0) strncpy(init, "s6", size);
    else strncpy(init, comm, size);
}

void get_gpu_info(char *gpu_name, size_t size) {
    char vendor_id[16] = "";
    char device_path[64];
    int found = 0;
    gpu_name[0] = '\0';
    for (int i = 0; i <= 1; i++) {
        snprintf(device_path, sizeof(device_path), "/sys/class/drm/card%d/device/vendor", i);
        FILE *fp = fopen(device_path, "r");
        if (fp) {
            if (fgets(vendor_id, sizeof(vendor_id), fp)) {
                char current_gpu[64];
                if (strstr(vendor_id, "0x10de")) strcpy(current_gpu, "NVIDIA [dGPU]");
                else if (strstr(vendor_id, "0x8086")) strcpy(current_gpu, "Intel [iGPU]");
                else if (strstr(vendor_id, "0x1002")) strcpy(current_gpu, "AMD [dGPU]");
                else if (strstr(vendor_id, "0x15ad")) strcpy(current_gpu, "VBox/VMware");
                else snprintf(current_gpu, sizeof(current_gpu), "GPU (%s)", vendor_id);
                if (found > 0) strncat(gpu_name, " | ", size - strlen(gpu_name) - 1);
                strncat(gpu_name, current_gpu, size - strlen(gpu_name) - 1);
                found++;
            }
            fclose(fp);
        }
    }
    if (found == 0) strncpy(gpu_name, "N/A", size);
}

int main() {
    char user[64], host[64], os[128], cpu[256], model[128], battery[64], gpu[128], init[64], display[64], de[64];
    struct sysinfo si;
    struct utsname un;
    struct statvfs vfs;

    getlogin_r(user, sizeof(user));
    gethostname(host, sizeof(host));
    uname(&un);
    get_init_system(init, sizeof(init));
    get_display_server(display, sizeof(display));
    get_de_wm(de, sizeof(de));
    get_gpu_info(gpu, sizeof(gpu));

    strncpy(os, "Linux", sizeof(os));
    FILE *f_os = fopen("/etc/os-release", "r");
    if (f_os) {
        char line[256];
        while (fgets(line, sizeof(line), f_os)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                char *name = line + 13;
                name[strcspn(name, "\"\n")] = 0;
                strncpy(os, name, sizeof(os));
            }
        }
        fclose(f_os);
    }

    read_sys_file("/sys/class/dmi/id/product_name", model, sizeof(model));
    
    cpu[0] = '\0';
    FILE *f_cpu = fopen("/proc/cpuinfo", "r");
    if (f_cpu) {
        char line[256];
        while (fgets(line, sizeof(line), f_cpu)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *name = strchr(line, ':') + 2;
                name[strcspn(name, "\n")] = 0;
                strncpy(cpu, name, sizeof(cpu));
                break;
            }
        }
        fclose(f_cpu);
    }

    char bat_cap[16] = "";
    read_sys_file("/sys/class/power_supply/BAT0/capacity", bat_cap, sizeof(bat_cap));
    if (strcmp(bat_cap, "N/A") != 0) snprintf(battery, sizeof(battery), "%s%%", bat_cap);
    else strcpy(battery, "Desktop/AC");

    statvfs("/", &vfs);
    unsigned long d_t = (vfs.f_blocks * vfs.f_frsize) / 1024 / 1024 / 1024;
    unsigned long d_u = ((vfs.f_blocks - vfs.f_bfree) * vfs.f_frsize) / 1024 / 1024 / 1024;

    sysinfo(&si);
    unsigned long m_u = (si.totalram - si.freeram) * si.mem_unit / 1024 / 1024;
    unsigned long m_t = si.totalram * si.mem_unit / 1024 / 1024;

    const char *arch_art[] = {"      /\\      ","     /  \\     ","    /    \\    ","   /      \\   ","  /   ,,   \\  "," /   |  |   \\ ","/_,,_/  \\_,,_\\","              "};
    const char *tux_art[] = {"    .--.      ","   |o_o |     ","   |:_/ |     ","  //   \\ \\    "," (|     | )   ","/'\\_   _/`\\   ","\\___)=(___/   ","              "};
    const char **art = (strstr(os, "Arch") || strstr(os, "Artix")) ? arch_art : tux_art;

    char info[INFO_LINES][256];
    int c = 0;
    snprintf(info[c++], 256, C_CYAN C_BOLD "%s" C_RESET "@" C_CYAN C_BOLD "%s" C_RESET, user, host);
    snprintf(info[c++], 256, "------------------");
    snprintf(info[c++], 256, C_CYAN "OS:      " C_RESET "%s", os);
    snprintf(info[c++], 256, C_CYAN "Model:   " C_RESET "%s", model);
    snprintf(info[c++], 256, C_CYAN "Kernel:  " C_RESET "%s", un.release);
    snprintf(info[c++], 256, C_CYAN "Display: " C_RESET "%s", display);
    snprintf(info[c++], 256, C_CYAN "DE/WM:   " C_RESET "%s", de);
    snprintf(info[c++], 256, C_CYAN "Init:    " C_RESET "%s", init);
    snprintf(info[c++], 256, C_CYAN "Uptime:  " C_RESET "%ldh %ldm", si.uptime/3600, (si.uptime%3600)/60);
    snprintf(info[c++], 256, C_CYAN "CPU:     " C_RESET "%s", cpu);
    snprintf(info[c++], 256, C_CYAN "GPU:     " C_RESET "%s", gpu);
    snprintf(info[c++], 256, C_CYAN "Memory:  " C_RESET "%luMiB / %luMiB", m_u, m_t);
    snprintf(info[c++], 256, C_CYAN "Disk:    " C_RESET "%luGB / %luGB", d_u, d_t);
    snprintf(info[c++], 256, C_CYAN "Battery: " C_RESET "%s", battery);
    snprintf(info[c++], 256, "");
    
    char blocks[256] = "";
    for(int i=0; i<8; i++) {
        char tmp[20];
        snprintf(tmp, sizeof(tmp), "\033[4%dm  ", i);
        strcat(blocks, tmp);
    }
    strcat(blocks, C_RESET);
    strncpy(info[c++], blocks, 256);

    int padding_top = (c > ASCII_LINES) ? (c - ASCII_LINES) / 2 : 0;

    printf("\n");
    for (int i = 0; i < c || i < ASCII_LINES; i++) {
        const char *line_art = "              ";
        if (i >= padding_top && i < padding_top + ASCII_LINES) {
            line_art = art[i - padding_top];
        }
        printf("  %-15s %s\n", line_art, (i < c) ? info[i] : "");
    }
    printf("\n");

    return 0;
}
