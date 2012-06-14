#include <string.h>
#include <jni.h>

jstring Java_de_unidue_tagrecognition_OpenCV_hello(JNIEnv* env, jobject javaThis) {
  return (*env)->NewStringUTF(env, "Hello from native code!");
}
