# Ultralytics YOLOv5, AGPL-3.0 license
from utils.general import cv2, non_max_suppression
from models.common import DetectMultiBackend
import torch
import numpy as np
from flask import Flask, render_template, request, jsonify, send_file
from flask_cors import CORS
import threading
import io
import cv2
import torch.nn as nn
import torchvision.transforms as transforms
from torchvision.models.segmentation import fcn_resnet50
import pymysql
import datetime

# ----------------------------------------------------------------------------------------------------------------------
# Model parameters
imgsz = (640, 640)  # inference size (height, width)
device = "cpu"  # cuda device, i.e. 0 or 0,1,2,3 or cpu

types = ['cross_1', 'cross_2', 'cross_3', 'cross_4', 'cross_5', 'cross_6', 'path_1', 'path_2', 'path_3', 'path_4', 'path_5', 'path_6', 'path_7']
figure_size = (224, 224)
arg_kwargs={
    'host':"localhost",
    'port':3308,
    'user':'nanoka',
    'password':"12345678n",
    'database':"manage",
    'charset':'utf8mb4'
}
# Cache
box_cache, yolo_cache, obj_cache = None, np.zeros([640, 640, 3]), np.zeros([640, 640, 3])

# MySQL handler
db = None
cursor = None
# YOLOv5 model handler
model = None
obj_model = None
# FCN model handler
fcn_model = None
transform = None
# OpenCV
cap = None



app = Flask(__name__)
CORS(app)


# ----------------------------------------------------------------------------------------------------------------------
def plot_xyxy(x, img, color=(0, 0, 255), label=None, line_thickness=3):
    c1, c2 = (int(x[0]), int(x[1])), (int(x[2]), int(x[3]))
    cv2.rectangle(img, c1, c2, color, thickness=line_thickness,
                  lineType=cv2.LINE_AA)

    if label:
        font_thickness = max(line_thickness - 1, 1)
        t_size = cv2.getTextSize(
            label, 0, fontScale=line_thickness / 3, thickness=font_thickness)[0]
        c2 = c1[0] + t_size[0], c1[1] - t_size[1] - 3
        cv2.rectangle(img, c1, c2, color, -1, cv2.LINE_AA)
        cv2.putText(img, label, (c1[0], c1[1] - 2), 0, line_thickness / 3,
                    [225, 255, 255], thickness=font_thickness, lineType=cv2.LINE_AA)


def inference(img):
    global yolo_cache
    ret = {'type': 'myCar', 'xyxy': [], 'conf': 0.0}

    # NHWC to NCHW
    yolo_cache = img.copy()
    image = yolo_cache.transpose(2, 0, 1)
    image = np.ascontiguousarray(image) / 255.0

    # Inference
    pred = model(torch.from_numpy(image).unsqueeze(0).float())

    # NMS algorithm
    pred = non_max_suppression(pred)

    # Process predictions
    for i, det in enumerate(pred):
        if len(det):
            # Write results
            for *xyxy, conf, cls in reversed(det):
                c = int(cls)  # integer class
                # Trained myCar type
                if c != 80:
                    continue

                if float(conf.item()) > ret['conf']:
                    ret['conf'] = float(conf.item())
                    ret['xyxy'] = [float(xyxy[i].item()) for i in range(4)]
                    
                    plot_xyxy(xyxy, yolo_cache, label='{:4f}'.format(float(conf.item())),
                              color=(0, 0, 255), line_thickness=2)
                    
                    point = [int((xyxy[0] + xyxy[2])/2), int((xyxy[1] + xyxy[3])/2)]
                    cv2.circle(yolo_cache, point, radius=5,
                            color=(255, 0, 0), thickness=10)
                    ret['point'] = point

    return ret

def object_split(img):
    global obj_cache
    ret = {'type': '', 'xyxy': [], 'conf': 0.0}
    types = ['red', 'blue', 'point']

    # NHWC to NCHW
    obj_cache = img.copy()
    image = obj_cache.transpose(2, 0, 1)
    image = np.ascontiguousarray(image) / 255.0

    # Inference
    pred = obj_model(torch.from_numpy(image).unsqueeze(0).float())

    # NMS algorithm
    pred = non_max_suppression(pred)

    # Process predictions
    for i, det in enumerate(pred):
        if len(det):
            # Write results
            for *xyxy, conf, cls in reversed(det):
                c = int(cls)  # integer class

                ret['type'] = types[c]
                if float(conf.item()) > ret['conf']:
                    ret['conf'] = float(conf.item())
                    ret['xyxy'] = [float(xyxy[i].item()) for i in range(4)]
                    
                    plot_xyxy(xyxy, obj_cache, label='{} {:4f}'.format(ret['type'], float(conf.item())),
                              color=(0, 0, 255), line_thickness=2)

    return ret

def val_vedio(cap):
    while True:
        _, frame = cap.read()
        frame = cv2.resize(frame, imgsz)

        # NHWC to NCHW
        image = frame.copy()
        image = image.transpose(2, 0, 1)
        image = np.ascontiguousarray(image) / 255.0

        # Inference
        pred = model(torch.from_numpy(image).unsqueeze(0).float())

        # NMS algorithm
        pred = non_max_suppression(pred)

        # Process predictions
        for i, det in enumerate(pred):
            if len(det):
                # Write results
                for *xyxy, conf, cls in reversed(det):
                    c = int(cls)  # integer class
                    # Trained myCar type
                    if c != 80:
                        continue

                    label = '{} {:4f}'.format(
                        model.names[c], float(conf.item()))

                    plot_xyxy(xyxy, frame, label=label,
                              color=(0, 0, 255), line_thickness=2)

                    # print("xyxy:", [float(xyxy[i].item())
                    #       for i in range(4)], "\tconf:", float(conf.item()))

            # Stream results
            cv2.imshow('nanoka_v1', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                cv2.destroyAllWindows()
                print("Done.")
                return
            
def fcn_write_box(im):
    global box_cache
    img = cv2.resize(im, figure_size, interpolation=cv2.INTER_LINEAR)
    # img = cv2.convertScaleAbs(img, alpha=1.8, beta=0.8)

    image = transform(img).unsqueeze(0)
    mask = None

    fcn_model.eval()
    with torch.no_grad():
        output = fcn_model(image)['out']
        pred = nn.LeakyReLU(negative_slope=0.01)(output)
        mask = pred[0].numpy()

    boxes = [[0,0,0,0] for _ in range(mask.shape[0])]
    for i in range(mask.shape[0]):
        mask_figure = cv2.threshold(mask[i, :, :], 8, 255, cv2.THRESH_BINARY)[1]

        contours, _ = cv2.findContours(mask_figure.astype(np.uint8), cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        buffer = np.zeros((224, 224), dtype=np.uint8)
        for contour in contours:
            x, y, w, h = cv2.boundingRect(contour)
            cv2.rectangle(buffer, (x, y), (x + w, y + h), 255, 3)
            
            boxes[i] = [x, y, w, h]

    box_img = img.copy()
    for i in range(len(boxes)):
        x, y, w, h = boxes[i]
        if i < 6:
            box_img = cv2.rectangle(box_img, (x, y), (x + w, y + h), (0, 0, 255), 2)
            box_img = cv2.putText(box_img, types[i], (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1, cv2.LINE_AA)
        else:
            box_img = cv2.rectangle(box_img, (x, y), (x + w, y + h), (255, 0, 0), 2)
            box_img = cv2.putText(box_img, types[i], (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 1, cv2.LINE_AA)

    box_cache = box_img
    return boxes

# ----------------------------------------------------------------------------------------------------------------------
# 响应成功回调函数
def success_callback(response):
    # 防御注入攻击
    sql = "INSERT INTO request(request, time) VALUES (0, %s)"
    values = (datetime.datetime.now())
    
    try:
        cursor.execute(sql, values)
        db.commit()
    except Exception:
        response.status_code = 500
        
    return response

# 响应失败回调函数
def failure_callback(response):
    # 防御注入攻击
    sql = "INSERT INTO request(request, time) VALUES (1, %s)"
    values = (datetime.datetime.now())
    
    try:
        cursor.execute(sql, values)
        db.commit()
    except Exception:
        response.status_code = 500
        
    return response

@app.after_request
def after_request(response):
    if response.status_code == 200:
        response = success_callback(response)
    else:
        response = failure_callback(response)
    return response

@app.route('/', methods=['GET'])
def root():
    return render_template('50x.html'), 500

@app.route('/image', methods=['GET'])
def image():
    _, frame = cap.read()
    frame = cv2.resize(frame, (640, 640))
    _, img_encoded = cv2.imencode('.jpg', frame)
    
    return send_file(io.BytesIO(img_encoded.tobytes()), mimetype='image/jpeg')


@app.route('/val', methods=['GET'])
def val():
    thread1 = threading.Thread(target=val_vedio, args=[1,])
    thread1.start()
    return jsonify({'code': 0, 'info': 'Success'}), 200

@app.route('/info', methods=['GET'])
def info():
    code = (0, 200)
    
    try:
        sql = "select COUNT(request) from request group by request;"
        cursor.execute(sql)
        db.commit()
        result = cursor.fetchall()
        percent = int(100 * result[0][0]/(result[0][0] + result[1][0]))
        
        sql = "SELECT DATE_FORMAT(time, '%Y-%m-%d %H:00:00') AS hour, COUNT(*) AS count FROM request WHERE time >= NOW() - INTERVAL 1 DAY GROUP BY hour ORDER BY hour DESC LIMIT 24;"
        cursor.execute(sql)
        db.commit()
        result = cursor.fetchall()
        
        time_str = [result[-i-1][0] for i in range(len(result))]
        x = [result[-i-1][1] for i in range(len(result))]
        
    except Exception as e:
        code = (1, 500)
        print(e)
    
    return jsonify({'code': code[0], 'info': percent, 'time_str': time_str, 'x' : x}), code[1]

@app.route('/fcn', methods=['POST'])
def fcn():
    if 'image' not in request.files:
        return jsonify({'code': 1, 'info': 'No image file'}), 400

    file = request.files['image']
    npimg = np.frombuffer(file.read(), np.uint8)
    img = cv2.imdecode(npimg, cv2.IMREAD_COLOR)

    box = fcn_write_box(img)
    return jsonify({'code': 0, 'info': box}), 200

@app.route('/yolo', methods=['GET'])
def yolo():
    _, frame = cap.read()
    frame = cv2.resize(frame, (640, 640))

    ret = inference(frame)
    return jsonify({'code': 0, 'info': ret}), 200

@app.route('/obj', methods=['GET'])
def object_detect():
    _, frame = cap.read()
    frame = cv2.resize(frame, (640, 640))

    ret = object_split(frame)
    return jsonify({'code': 0, 'info': ret}), 200

@app.route('/fcn_image', methods=['GET'])
def fcn_image():
    global box_cache
    frame = cv2.resize(box_cache, (640, 640))
    _, img_encoded = cv2.imencode('.jpg', frame)
    
    return send_file(io.BytesIO(img_encoded.tobytes()), mimetype='image/jpeg')

@app.route('/yolo_image', methods=['GET'])
def yolo_image():
    global yolo_cache
    frame = cv2.resize(yolo_cache, (640, 640))
    _, img_encoded = cv2.imencode('.jpg', frame)
    
    return send_file(io.BytesIO(img_encoded.tobytes()), mimetype='image/jpeg')

@app.route('/obj_image', methods=['GET'])
def obj_image():
    global obj_cache
    frame = cv2.resize(obj_cache, (640, 640))
    _, img_encoded = cv2.imencode('.jpg', frame)
    
    return send_file(io.BytesIO(img_encoded.tobytes()), mimetype='image/jpeg')

@app.route('/analyze', methods=['POST'])
def analyze():
    if 'image' not in request.files:
        return jsonify({'code': 1, 'info': 'No image file'}), 400

    file = request.files['image']
    npimg = np.frombuffer(file.read(), np.uint8)
    img = cv2.imdecode(npimg, cv2.IMREAD_COLOR)

    ret = inference(img)
    xyxy = ret['xyxy']
    conf = ret['conf']

    data = [0, 0]

    if conf > 0.3:
        data = [(xyxy[0] + xyxy[2])/2, (xyxy[1] + xyxy[3])/2]
        return jsonify({'code': 0, 'info': 'Success', 'data': str(data)}), 200
    else:
        return jsonify({'code': 2, 'info': 'No myCar'}), 200


# ----------------------------------------------------------------------------------------------------------------------
if __name__ == "__main__":
    # Load YOLOv5 model (myCar)
    model = DetectMultiBackend('nanoka_v1.pt', data='dataset/nanoka_v1/data.yaml')
    # Load YOLOv5 model (Object detect)
    obj_model = DetectMultiBackend('nanoka_v2.pt', data='dataset/nanoka_v2/data.yaml')
    
    # Load FCN_Resnet50 model
    fcn_model = fcn_resnet50(pretrained=True)
    fcn_model.classifier[4] = nn.Conv2d(512, 13, kernel_size=1)
    fcn_model.load_state_dict(torch.load('fcn_model.pth'))

    transform = transforms.Compose([
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
    ])
    
    # Load MySQL connection
    db = pymysql.connections.Connection(**arg_kwargs)
    cursor = db.cursor()

    # Load camera
    cap = cv2.VideoCapture(0)
    
    # Run Flask serve
    app.run('0.0.0.0', port=5000)
