# Server guideline
You can run any model you want with QNN. Here, I'll demonstrate converting a model from [ai-hub-models](https://github.com/quic/ai-hub-models) and port it into Android application, especially YOLOv6. If you want to convert other models in ai-hub-models, please refer to that model's README.

### 1. Install requirements of ai-hub-models
```
# Install requirements
pip install qai_hub_models
pip install ultralytics         # Package for Yolo series
```
<br/>

### 2. Convert the models you want as `.tflite`. 
I choose `.tflite` since this sample app also supports running the models as TFLite(LiteRT) using `QNNDelegate`.

```python
# Convert the model to your format (e.g., ONNX, TFLite, ...)
python -m qai_hub_models.models.yolov6.export \
    --target-runtime tflite \
    --device "Samsung Galaxy S23" \     # Your target device
    --height 480 \      # To compatible with QNN-Android implementation
    --width 640 \       # To compatible with QNN-Android implementation
    --output-dir converted_models
```
After the command, the model would be converted at `converted_models/` (e.g., `converted_models/yolov6.tflite`).   
<br/>

### 3. Convert the model from #2 to QNN format.
i.e., Generate `model.cpp` and `model.bin` using tools provided from QNN. Use tools and options you want.
```bash
qnn-tflite-converter -i <MODEL_PATH> -d <INPUT_NAME> <INPUT_SHAPE> -o <OUTPUT_DIR> ...
```

Below is the example command that convert `yolov6.tflite` to QNN.
```bash
qnn-tflite-converter -i converted_models/yolov6.tflite -d "image" "1,480,640,3" -o converted_models/yolov6.cpp
```
After the command,  the model would be converted as `converted_models/yolov6.cpp` and `converted_models/yolov6.bin`.  

<br/>

### 4. Build model library as `.so` by using `qnn-model-lib-generator`. 
Besides, I'm seeking for the way to load `.cpp` and `.bin` directly in Android code.
```bash
qnn-model-lib-generator -c converted_models/yolov6.cpp -b converted_models/yolov6.bin -l YOLOv6_FP16.so -o converted_models/ -t aarch64-android
```
After the command, the library would be generated at `converted_models/aarch64-android/`.  
<br/>

### 5. Put the library file into the `app/src/main/jniLibs/arm64-v8a/`. 
