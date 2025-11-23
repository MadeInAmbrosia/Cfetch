package main

import (
	"fmt"
	"os"
	"os/exec"
	"strings"
)

func runCommand(command string) string {
	out, err := exec.Command("sh", "-c", command).Output()
	if err != nil {
		return ""
	}
	return strings.TrimSpace(string(out))
}

func getIconTheme() string {
	home := os.Getenv("HOME")
	if home == "" {
		return "Unknown"
	}

	gtkPath := fmt.Sprintf("%s/.config/gtk-3.0/settings.ini", home)
	if _, err := os.Stat(gtkPath); err == nil {
		cmd := "grep -oP '(?<=gtk-icon-theme-name=).*' " + gtkPath
		icon := runCommand(cmd)
		if icon != "" {
			return icon
		}
	}

	kdePath := fmt.Sprintf("%s/.config/kdeglobals", home)
	if _, err := os.Stat(kdePath); err == nil {
		cmd := "grep -oP '(?<=Icons=).*' " + kdePath
		icon := runCommand(cmd)
		if icon != "" {
			return icon
		}
	}

	return "Unknown"
}

func getGPUs() string {
	gpus := runCommand("lspci | grep -E 'VGA|3D' | awk -F ': ' '{print $2}'")
	if gpus == "" {
		return "Unknown"
	}

	gpuList := strings.Split(gpus, "\n")
	var gpuOutput string
	for i, gpu := range gpuList {
		gpuOutput += fmt.Sprintf("GPU %d: %s\n", i+1, gpu)
	}
	return gpuOutput
}

func getWMOrDE() string {
	de := os.Getenv("XDG_CURRENT_DESKTOP")
	if de == "" {
		de = runCommand("xprop -root | grep '_NET_WM_NAME' | cut -d' ' -f3-")
		if de == "" {
			de = "Unknown"
		}
	}

	wms := []string{"i3", "bspwm", "sway", "openbox", "awesome", "herbstluftwm", "dwm", "fluxbox", "ratpoison"}
	for _, wm := range wms {
		if strings.Contains(de, wm) {
			return "WM: " + wm
		}
	}

	return "DE: " + de
}

func main() {
	fmt.Println("\n▄█████  ▄▄▄  ██████ ██████ ██████ ▄█████ ██  ██ ")
	fmt.Println("██     ██▀██ ██▄▄   ██▄▄     ██   ██     ██████ ")
	fmt.Println("▀█████ ██▀██ ██     ██▄▄▄▄   ██   ▀█████ ██  ██ \n")

	osDistro := runCommand("uname -s")
	var distro string
	if osDistro == "Linux" {
		distro = runCommand("lsb_release -d | awk -F'\t' '{print $2}'")
		if distro == "" {
			distro = "Linux"
		}
	} else if osDistro == "Darwin" {
		distro = "macOS"
	} else {
		distro = "Windows"
	}

	de := getWMOrDE()
	shell := os.Getenv("SHELL")
	if shell == "" {
		shell = "Unknown"
	}

	cpu := runCommand("lscpu | grep 'Model name' | awk -F ':' '{print $2}'")
	gpus := getGPUs()
	memory := runCommand("free -h | awk '/Mem:/ {print $3\" / \"$2\" in use\"}'")
	uptime := runCommand("uptime -p")
	kernel := runCommand("uname -r")
	host := runCommand("hostnamectl | grep 'Operating System' | awk -F ':' '{print $2}'")
	terminal := os.Getenv("TERM")
	if terminal == "" {
		terminal = "Unknown"
	}
	icon := getIconTheme()

	fmt.Printf("OS: %s\n", distro)
	fmt.Printf("%s\n", de)
	fmt.Printf("Shell: %s\n", shell)
	fmt.Printf("CPU: %s\n", cpu)
	fmt.Print(gpus)
	fmt.Printf("Memory: %s\n", memory)
	fmt.Printf("Host: %s\n", host)
	fmt.Printf("Terminal: %s\n", terminal)
	fmt.Printf("Icon Theme: %s\n", icon)
	fmt.Printf("Uptime: %s\n", uptime)
	fmt.Printf("Kernel: %s\n", kernel)
}
