FROM archlinux:latest

RUN pacman -Syyu --noconfirm

RUN pacman -S --noconfirm sed git gcc make

COPY . /lidar-vis

WORKDIR /lidar-vis

RUN ./install_sfml

RUN ./install_rplidar

RUN make lidarvis

RUN LD_LIBRARY_PATH=/lidar-vis/sfml/lib

CMD [ "./lidarvis" "--gui", "0", "--file" "datasets/knei-1.txt" ]
