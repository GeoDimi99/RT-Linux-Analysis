FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y python3-pip 
# Set up Python API
WORKDIR /app/api
COPY api/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY api .


# Start both applications
CMD sh -c "cd /app/api && python3 app.py"
