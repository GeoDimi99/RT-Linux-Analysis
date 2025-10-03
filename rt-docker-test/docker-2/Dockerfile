FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y build-essential 

WORKDIR /app/api
COPY api .

# Start both applications
CMD sh -c "cd /app/api && make && ./rt-task 5"
