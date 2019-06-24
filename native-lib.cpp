#include <native-lib.h>

extern "C" JNIEXPORT jstring JNICALL

Java_com_caration_mangu_activitys_operationTizheng(
        JNIEnv *env,
        jobject /* this */,jstring file,jstring cmd) {
    int fd = open(env->GetStringUTFChars(file,NULL),O_RDWR);
    const char* s= env->GetStringUTFChars(cmd,NULL);
    write(fd,s,sizeof(s));
	close(fd);
    //return env->NewStringUTF("");

}


