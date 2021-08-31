FROM    ubuntu
WORKDIR /luos_gatt

ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime \
    && echo $TZ > /etc/timezone

# Mandatory packages
RUN     apt update && apt install -y    \
            vim                 \
            wget                \
            unzip               \
            cmake               \
            gcc-arm-none-eabi   \
            git

# Install JLink software
RUN     wget --post-data="accept_license_agreement=accepted&submit=Download&nbspsoftware" https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb -O JLink.deb    \
            && dpkg -i JLink.deb    \
            && rm -f JLink.deb      \
            && apt update

# Install Nordic command line tools
RUN     wget https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-12-1/nRFCommandLineTools10121Linuxamd64.tar.gz -O cl_tools.tar.gz    \
            && mkdir cl_tools                           \
            && tar -xzvf cl_tools.tar.gz -C cl_tools    \
            && dpkg -i cl_tools/nRF*.deb                \
            && rm -r cl_tools*                          \
            && apt update

# Install NRF5 SDK
RUN     wget https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5SDK153059ac345.zip -O SDK.zip   \
            && unzip SDK.zip    \
            && mv nRF* SDK      \
            && rm SDK.zip

# Copy build context
COPY resources      /luos_gatt/resources
COPY gate_node      /luos_gatt/gate_node
COPY actuator_node  /luos_gatt/actuator_node
COPY build.sh       /luos_gatt/build.sh
COPY flash.sh       /luos_gatt/flash.sh

CMD ["/bin/bash"]
