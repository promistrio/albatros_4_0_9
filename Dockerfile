FROM ubuntu:16.04
WORKDIR /ardupilot

ARG DEBIAN_FRONTEND=noninteractive

RUN useradd -U -m ardupilot && \
    usermod -G users ardupilot

RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install --no-install-recommends -y \
    lsb-release \
    sudo \
    software-properties-common \
    python-software-properties

COPY Tools/environment_install/install-prereqs-ubuntu.sh /ardupilot/Tools/environment_install/
COPY Tools/environment_install/get-pip.py /ardupilot/Tools/environment_install/

RUN Tools/environment_install/get-pip.py

# install this to use gui with vnc
# RUN apt-get install -y x11vnc xvfb fluxbox
# RUN sudo apt-get install -y flightgear

ENV USER=ardupilot
RUN echo "ardupilot ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/ardupilot
RUN chmod 0440 /etc/sudoers.d/ardupilot
RUN chown -R ardupilot:ardupilot /ardupilot

USER ardupilot

RUN echo "alias waf=\"/ardupilot/waf\"" >> ~/.bashrc
RUN echo "if [ -d \"\$HOME/.local/bin\" ] ; then\nPATH=\"\$HOME/.local/bin:\$PATH\"\nfi" >> ~/.bashrc
RUN Tools/environment_install/install-prereqs-ubuntu.sh -y

ENV BUILDLOGS=/tmp/buildlogs

RUN sudo apt-get clean \
    && sudo rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ENV CCACHE_MAXSIZE=1G
# ENV PATH /usr/lib/ccache:/ardupilot/Tools:/ardupilot/Tools/autotest:/opt/gcc-arm-none-eabi-6-2017-q2-update:${PATH}
