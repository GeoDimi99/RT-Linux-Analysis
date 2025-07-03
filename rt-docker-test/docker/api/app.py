from flask import Flask, jsonify
import time

app = Flask(__name__)

@app.route('/')
def hello_world():
    return jsonify({"message": "Hello from Flask RT API"})

@app.route('/ping')
def ping():
    return jsonify({
        "timestamp": time.time(),
        "status": "ok",
        "server": "Flask"
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
