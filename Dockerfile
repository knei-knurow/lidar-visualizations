FROM alpine:latest

RUN apk --no-cache add curl

COPY . /lidar-vis

WORKDIR /lidar-vis

RUN ./install_sfml

RUN ./install_rplidar

RUN make

RUN LD_LIBRARY_PATH=/lidar-vis/sfml/lib

CMD [ "./lidarvis" "-fs" "datasets/series/pokoj-podnoszenie-gora-dol.txt"]
