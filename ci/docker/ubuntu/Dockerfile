# Create a virtual environment with all tools installed
# ref: https://hub.docker.com/_/ubuntu
FROM ubuntu:20.04 AS base
LABEL maintainer="x.chen0724@gmail.com"

# Let us install tzdata painlessly
ENV DEBIAN_FRONTEND=noninteractive

# Needed for string substitution
SHELL ["/bin/bash", "-c"]

# Install system build dependencies
# note: here we use the CMake package provided by Ubuntu
# see: https://repology.org/project/cmake/versions
RUN apt-get update -q && \
apt-get install -yq git build-essential cmake && \
apt-get clean && \
rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Install dependencies
SHELL ["/bin/bash", "-c"]
RUN apt-get update -q && \
    apt-get install -yq libncurses5-dev libncursesw5-dev