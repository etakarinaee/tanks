
# Install

Debian:
```bash
sudo apt update
sudo apt install libfreetype6-dev libglfw3-dev portaudio19-dev luaji
```

Arch:
```bash
sudo pacman -S freetype2 glfw portaudio luajit
```

# Build 
```
git clone https://github.com/etakarinaee/sausages.git
cd sausage
mkdir build
cd build
cmake ..
make
```

# Usage
```bash
# Server 
SAUSAGES_IP=<ipv4 default -> 127.0.0.1> ./server

# Client
SAUSAGES_IP=<ipv4 default -> 127.0.0.1> SAUSAGES_NICKNAME=<nickname default -> Player> ./client
```

# gallery

<img width="1110" height="663" alt="image" src="https://github.com/user-attachments/assets/5825d6f9-c405-464b-ba16-fb5b8dff3215" />

