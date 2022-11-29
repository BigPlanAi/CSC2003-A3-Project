from flask import Flask, render_template, request
import random
import time

app = Flask(__name__)

carStatus = False
carCommand = "0"

@app.route("/api/test/<name>/")
def test(name):
    print(len(name))
    return "1 "


@app.route("/api/command/<name>")
def command2(name):
    print(name)
    return {"success": True, "command": name}


# Car will constantly call this for car status
@app.route("/api/start")
def start():
    if carStatus == False:
        return "0"
    else:
        return "1"

count = 0
startTime = None
@app.route("/api/tp")
def tp():
    global count, startTime
    count += 1
    if count == 1:
        startTime = time.time()
    elif count == 5:
        end = time.time()
        print(str(end-startTime))
    print(count)
    return "1"

reliabilityCount = 0
@app.route("/api/reliability<name>")
def latency(name):
    global reliabilityCount
    print(reliabilityCount)
    reliabilityCount2 = reliabilityCount % 5
    if (reliabilityCount2 == 0):
        if name != "a":
            print(f"{reliabilityCount} | Loop Broked")
    elif (reliabilityCount2 == 1):
        if name != "b":
            print(f"{reliabilityCount} | Loop Broked")
    elif (reliabilityCount2 == 2):
        if name != "c":
            print(f"{reliabilityCount} | Loop Broked")
    elif (reliabilityCount2 == 3):
        if name != "d":
            print(f"{reliabilityCount} | Loop Broked")
    elif (reliabilityCount2 == 4):
        if name != "e":
            print(f"{reliabilityCount} | Loop Broked")
    
# Web app control
@app.route("/commands", methods=["POST"])
def commands():
    global carStatus
    command = request.get_json()
    if command["method"] == "start":
        print(carStatus)
        if carStatus == False:
            carStatus = True
            # publish(client,"1#")
            return {"success": True}
        else:
            return {"success": False}, 400

    elif command["method"] == "stop":
        print(carStatus)
        if carStatus == True:
            carStatus = False
            # publish(client,"2#")
            return {"success": True}
        else:
            return {"success": False}, 400

    
    elif command["method"] == "left":
        command = "1"
        return {"success": True}
    return {"success": True}

# Car will call this constantly to get commands
@app.route("/api/getCommands")
def getCommands():
    # 0 = Nothing
    # 1 = Turn Left
    # 2 = Turn Right
    return "01001011"

@app.route("/")
@app.route("/index")
def index():
    with open("data.txt", "r") as f:
        msp = f.read()
    return render_template("index.html", title="Welcome", data=msp)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=105)
