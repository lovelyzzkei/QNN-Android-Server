<h1 align='center'> Let's use Qualcomm NPU in Android </h1>

<h3 align='center'> Running your model on mobile devices using NPU </h3>

- Easily create QNN/TFLite models on the server
- A simple app to test a converted model on Android.
- Support FP16/INT8 inference of QNN.

The Goal of this project is to provide guidelines and a sample Android app to use Qualcomm QNN easily. ***Currently, I only tested running QNN model by FP16. I'll keep maintaining this code to support some other DNN models and other arithmetic precisions.***

## Prerequiste
- Qualcomm® AI Engine Direct setup should be completed by following the guide [here](https://docs.qualcomm.com/bundle/publicresource/topics/80-63442-50/setup.html).
- Android device 6.0 and above can be used to test the application.
- If you want to use TFLite, you have to move your TFLite libraries in `QNN-Android/app/libs`

## Source Overview
### Source Organization
- `QNN-Android`: Simple android demo app code to run QNN model. It's kind of skeleton code.
    - `QNN-Android/app/src/main/cpp/QnnManager.cpp`: Manager code that has all codes(Initializing to Running) related to QNN model running.
- `Server`: Simple model conversion guidelines using Qualcomm AI Hub.

If you want more specific meanings of each codes, please visit [here](https://docs.qualcomm.com/bundle/publicresource/topics/80-63442-50/sample_app.html?vproduct=1601111740013072&version=1.1&facet=Qualcomm%20AI%20Engine%20Direct%20SDK)