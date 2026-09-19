# Use an official Ubuntu base image
FROM ubuntu:latest

# Install git and clean up apt cache
RUN apt-get update && apt-get install -y git build-essential valgrind \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /app

# You can add other setup here, like copying your project files
# COPY . .

# Command to run when the container starts
CMD ["/bin/bash"]
