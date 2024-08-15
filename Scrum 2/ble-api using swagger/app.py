from flask import Flask, jsonify, request
from flask_swagger_ui import get_swaggerui_blueprint

app = Flask(__name__)

# Data Interface Config
@app.route('/ble/data_interface/read', methods=['GET'])
def read_data_interface():
    data = {
        "type_connection": "RTU",
        "data_type": "Coils",
        "id": 1,
        "address": "0x01",
        "value": "0xFF"
    }
    return jsonify(data)

@app.route('/ble/data_interface/write', methods=['POST'])
def write_data_interface():
    data = request.get_json()
    if data:
        print("Updating Data Interface Config with:", data)
        return jsonify({"message": "Data interface config updated"}), 200
    else:
        return jsonify({"error": "Invalid data"}), 400

# Communication Interface Config
@app.route('/ble/comm_interface/read', methods=['GET'])
def read_comm_interface():
    data = {
        "connection_method": "Ethernet",
        "mac_address": "00:1B:44:11:3A:B7",
        "ip_address": "192.168.1.100"
    }
    return jsonify(data)

@app.route('/ble/comm_interface/write', methods=['POST'])
def write_comm_interface():
    data = request.get_json()
    if data:
        print("Updating Communication Interface Config with:", data)
        return jsonify({"message": "Communication interface config updated"}), 200
    else:
        return jsonify({"error": "Invalid data"}), 400

# Data Store Configuration
@app.route('/ble/data_store/read', methods=['GET'])
def read_data_store():
    data = {
        "storage_method": "MQTT",
        "mqtt_broker": "broker.hivemq.com",
        "mqtt_topic": "/sensor/data"
    }
    return jsonify(data)

@app.route('/ble/data_store/write', methods=['POST'])
def write_data_store():
    data = request.get_json()
    if data:
        print("Updating Data Store Config with:", data)
        return jsonify({"message": "Data store config updated"}), 200
    else:
        return jsonify({"error": "Invalid data"}), 400

# Logging
@app.route('/ble/logging/read', methods=['GET'])
def read_logging():
    data = {
        "log_type": "Data",
        "log_interval": 60000000
    }
    return jsonify(data)

@app.route('/ble/logging/write', methods=['POST'])
def write_logging():
    data = request.get_json()
    if data:
        print("Updating Logging Config with:", data)
        return jsonify({"message": "Logging config updated"}), 200
    else:
        return jsonify({"error": "Invalid data"}), 400

# Swagger setup
SWAGGER_URL = '/swagger'
API_URL = '/static/suriota.json'
swaggerui_blueprint = get_swaggerui_blueprint(SWAGGER_URL, API_URL, config={'app_name': "BLE API"})
app.register_blueprint(swaggerui_blueprint, url_prefix=SWAGGER_URL)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
