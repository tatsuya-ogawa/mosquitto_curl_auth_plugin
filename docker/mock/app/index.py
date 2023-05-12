from flask import *
app = Flask(__name__)

@app.route("/auth",methods=["GET", "POST"])
def auth():
    username = request.form["username"]
    password = request.form["password"]
    ip = request.form["ip"]
    print(f"username:{username},password:{password},ip:{ip}",flush=True)
    if username == "hoge" and password == "huga":
        return "{}",200
    else:
        return "{}",401

if __name__ == "__main__":
  app.run(host="0.0.0.0", port=80, debug=True)