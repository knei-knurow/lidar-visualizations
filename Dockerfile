FROM gcc:latest AS build
ADD . /home/lidar
WORKDIR /home/lidar
RUN make lidarvis
FROM gcc:latest
COPY --from=build /home/lidar/lidarvis /bin/lidarvis
ENTRYPOINT ["/bin/lidarvis", "-g", "0"]