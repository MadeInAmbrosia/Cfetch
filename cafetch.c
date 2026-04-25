#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 128
#define WIDTH 15

void run_command(const char *cmd, char *output, size_t size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        output[0] = '\0';
        return;
    }
    fgets(output, size, fp);
    output[strcspn(output, "\n")] = 0;
    pclose(fp);
}

int get_gpu_lines(char gpus[MAX_LINES][256]) {
    FILE *fp = popen("lspci | grep -E 'VGA|3D'", "r");
    if (!fp) return 0;

    int count = 0;
    char line[512];

    while (fgets(line, sizeof(line), fp) && count < MAX_LINES) {

        char *gpu = strstr(line, ": ");
        if (!gpu) continue;
        gpu += 2;

        gpu[strcspn(gpu, "\n")] = 0;

        char name[256];
        strncpy(name, gpu, sizeof(name));
        name[sizeof(name) - 1] = 0;

        char vendor[64] = "";
        char type[32] = "GPU";

        if (strstr(name, "NVIDIA")) strcpy(vendor, "NVIDIA ");
        else if (strstr(name, "Intel")) strcpy(vendor, "Intel ");
        else if (strstr(name, "AMD") || strstr(name, "ATI")) strcpy(vendor, "AMD ");

        if (strstr(name, "Integrated") ||
            strstr(name, "iGPU") ||
            strstr(name, "HD Graphics") ||
            strstr(name, "Radeon Graphics") ||
            strstr(name, "UMA")) {
            strcpy(type, "Integrated");
        } else {
            strcpy(type, "Discrete");
        }

        char *clean = name;

        char *amd = strstr(clean, "Advanced Micro Devices, Inc. [AMD/ATI]");
        if (amd) clean = amd + strlen("Advanced Micro Devices, Inc. [AMD/ATI] ");

        snprintf(gpus[count], 256, "GPU %d: %s%s [%s]",
                 count + 1, vendor, clean, type);

        count++;
    }

    pclose(fp);

    if (count == 0) {
        strcpy(gpus[0], "GPU: Unknown");
        return 1;
    }

    return count;
}

int main() {
    const char *ascii[] = {
        "    .--.",
        "   |o_o |",
        "   |:_/ |",
        "  //   \\ \\",
        " (|     | )",
        "/'\\_   _/`\\",
        "\\___)=(___/"
    };

    int ascii_lines = sizeof(ascii) / sizeof(ascii[0]);

    char distro[128] = "Linux";
    run_command("lsb_release -d | awk -F'\t' '{print $2}'", distro, sizeof(distro));

    char shell[128];
    char *env_shell = getenv("SHELL");
    strcpy(shell, env_shell ? env_shell : "Unknown");

    char cpu[256];
    run_command("lscpu | grep 'Model name' | awk -F ':' '{print $2}' | sed 's/^ *//'", cpu, sizeof(cpu));

    char memory[128];
    run_command("free -h | awk '/Mem:/ {print $3\" / \"$2\" in use\"}'", memory, sizeof(memory));

    char uptime[128];
    run_command("uptime -p", uptime, sizeof(uptime));

    char kernel[128];
    run_command("uname -r", kernel, sizeof(kernel));

    char terminal[128];
    char *env_term = getenv("TERM");
    strcpy(terminal, env_term ? env_term : "Unknown");

    char gpus[MAX_LINES][256];
    int gpu_count = get_gpu_lines(gpus);

    char *info[MAX_LINES];
    int info_count = 0;

    static char os_line[256];
    snprintf(os_line, sizeof(os_line), "OS: %s", distro);
    info[info_count++] = os_line;

    static char shell_line[256];
    snprintf(shell_line, sizeof(shell_line), "Shell: %s", shell);
    info[info_count++] = shell_line;

    static char cpu_line[256];
    snprintf(cpu_line, sizeof(cpu_line), "CPU: %s", cpu);
    info[info_count++] = cpu_line;

    for (int i = 0; i < gpu_count; i++) {
        info[info_count++] = gpus[i];
    }

    static char mem_line[256];
    snprintf(mem_line, sizeof(mem_line), "Memory: %s", memory);
    info[info_count++] = mem_line;

    static char up_line[256];
    snprintf(up_line, sizeof(up_line), "Uptime: %s", uptime);
    info[info_count++] = up_line;

    static char kern_line[256];
    snprintf(kern_line, sizeof(kern_line), "Kernel: %s", kernel);
    info[info_count++] = kern_line;

    static char term_line[256];
    snprintf(term_line, sizeof(term_line), "Terminal: %s", terminal);
    info[info_count++] = term_line;

    int total_lines = info_count > ascii_lines ? info_count : ascii_lines;
    int ascii_offset = (total_lines - ascii_lines) / 2;

    for (int i = 0; i < total_lines; i++) {
        const char *left = "";
        const char *right = "";

        if (i >= ascii_offset && i < ascii_offset + ascii_lines) {
            left = ascii[i - ascii_offset];
        }
        if (i < info_count) {
            right = info[i];
        }

        printf("%-*s %s\n", WIDTH, left, right);
    }

    return 0;
}
