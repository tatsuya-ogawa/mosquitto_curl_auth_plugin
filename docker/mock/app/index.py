from flask import *
import json
app = Flask(__name__)

@app.route("/auth",methods=["GET", "POST"])
def auth():
    username = request.form["username"]
    password = request.form["password"]
    ip = request.form["ip"]
    print(f"username:{username},password:{password},ip:{ip}",flush=True)
    if username == "hoge" and password == "fuga":
        return json.dumps({
            "topic_patterns":[
                "hoge/+/fuga",
                "abc/#"
            ]
        }),200
    else:
        return json.dumps({}),401

if __name__ == "__main__":
  app.run(host="0.0.0.0", port=80, debug=True)