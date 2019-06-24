#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <jni.h>
#include <sys/stat.h>
#include "include/turbojpeg.h"
#include "include/jconfig.h"
#include "include/jconfigint.h"

#include <android/log.h>
// 宏定义类似java 层的定义,不同级别的Log LOGI, LOGD, LOGW, LOGE, LOGF。 对就Java中的 Log.i log.d
#define LOG_TAG    "hpc -- JNILOG" // 这个是自定义的LOG的标识
//#undef LOG // 取消默认的LOG
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG, __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, __VA_ARGS__)
#define LOGF(...)  __android_log_print(ANDROID_LOG_FATAL,LOG_TAG, __VA_ARGS__)

typedef unsigned char uchar;
typedef struct tjp_info {
	int outwidth;
	int outheight;
	unsigned long jpeg_size;
}tjp_info_t;

static int get_timer_now();
uchar *read_file2buffer(char *filepath,tjp_info_t *tinfo);
void write_bufer2file(char *filepath,uchar *buffer,int size);
uchar *tjpeg_decompress(uchar *jpg_buffer,tjp_info_t *tinfo);
int tjpeg_compress(uchar *rgb_buffer,tjp_info_t *tinfo,int quality,uchar **outjpg_buffer,unsigned long *outjpg_size);
int tj_test();

static int get_timer_now(){

	struct timeval nows;
	gettimeofday(&nows,NULL);
	return(nows.tv_sec*1000 + nows.tv_usec/1000);
}

uchar *read_file2buffer(char *filepath,tjp_info_t *tinfo){
	FILE *fd;
	struct stat fileinfo;
	stat(filepath,&fileinfo);
	tinfo->jpeg_size = fileinfo.st_size;

	fd = fopen(filepath,"rb");
	if(NULL == fd){
		LOGD("file not open \n");
		return NULL;
	}

	uchar *data = (uchar *)malloc(sizeof(uchar) * fileinfo.st_size);
	fread(data,1,fileinfo.st_size,fd);
	fclose(fd);
	return data;
}

void write_bufer2file(char *filepath,uchar *buffer,int size){
	FILE *fd = fopen(filepath,"wb");
	if(NULL == fd){
		return;
	}
	fwrite(buffer,1,size,fd);
	fclose(fd);

}

uchar *tjpeg_decompress(uchar *jpg_buffer,tjp_info_t *tinfo){
	tjhandle handle = NULL;
	int img_width,img_height,img_subsamp,img_colorspace;
	int flags = 0,pixlfmt = TJPF_RGB;
	handle = tjInitDecompress();
	if(NULL == handle){
		return NULL;
	}

	int ret = tjDecompressHeader3(handle,jpg_buffer,tinfo->jpeg_size,&img_width,&img_height,&img_subsamp,&img_colorspace);
	if(0 != ret){
		tjDestroy(handle);
		return NULL;
	}
	LOGD("jpeg width:%d\n",img_width); 
	LOGD("jpeg height:%d\n",img_height); 
	LOGD("jpeg subsamp:%d\n",img_subsamp); 
	LOGD("jpeg colorspace:%d\n",img_colorspace);
	/*计算1/4缩放后的图像大小,若不缩放，那么直接将上面的尺寸赋值给输出尺寸*/
	tjscalingfactor sf;
	sf.num = 1;
	sf.denom = 2;
	tinfo->outwidth = TJSCALED(img_width,sf);
	tinfo->outheight = TJSCALED(img_height,sf);
	LOGD("w:%d,h:%d\n",tinfo->outwidth,tinfo->outheight);
	flags |= 0;
	int size = tinfo->outwidth * tinfo->outheight *3;
	uchar *rgb_buffer = (uchar *)malloc(sizeof(uchar)* size);
	/*解压缩时，tjDecompress2（）会自动根据设置的大小进行缩放，但是设置的大小要在它的支持范围，如1/2 1/4等*/
	ret = tjDecompress2(handle,jpg_buffer,tinfo->jpeg_size,rgb_buffer, tinfo->outwidth, 0,tinfo->outheight, pixlfmt, flags)
	;if(0!=ret){
		tjDestroy(handle);
		return NULL;
	}
	tjDestroy(handle);
	return rgb_buffer;
}


int tjpeg_compress(uchar *rgb_buffer,tjp_info_t *tinfo,int quality,uchar **outjpg_buffer,unsigned long *outjpg_size){
	tjhandle handle = NULL;
	int flags = 0;
	int subsamp = TJSAMP_422;
	int pixfmt = TJPF_RGB;
	handle = tjInitCompress();
	if(NULL == handle){
		return -1;
	}

	int ret = tjCompress2(handle,rgb_buffer,tinfo->outwidth,0,tinfo->outheight,pixfmt,outjpg_buffer,outjpg_size,subsamp,quality,flags);
	if( 0 != ret){
		tjDestroy(handle);
		return -1;
	}
	tjDestroy(handle);
	return 0;

}

int tj_test(){
	
	tjp_info_t tinfo;
	char *filename = "sdcard/test.jpg";
	int start = get_timer_now();
	uchar *jpeg_buffer = read_file2buffer(filename,&tinfo);
	int rend = get_timer_now();
	if(NULL == jpeg_buffer){
		return -1;
	}
	int dstart = get_timer_now();
	uchar *rgb = tjpeg_decompress(jpeg_buffer,&tinfo);
	if(NULL == rgb){
		free(jpeg_buffer);
		return -1;
	}
	int dend = get_timer_now();
	uchar *out_jpeg = NULL;
	unsigned long outjpegsize;
	int cstart = get_timer_now();
	tjpeg_compress(rgb,&tinfo,80,&out_jpeg,&outjpegsize);
	int cend = get_timer_now();
	char *outfile = "sdcard/out.jpg";
	write_bufer2file(outfile,out_jpeg,outjpegsize);
	
	free(jpeg_buffer);
	free(rgb);
	return 0;
}

void Java_com_lincy_charging2_MainActivity_stringFromJNI(
        JNIEnv* env,jobject bo) {
		int wstart = get_timer_now();
		int result = tj_test();
		int end = get_timer_now();
		LOGD("write file make time:%d\n",end-wstart);
		LOGD(" fufufufufufu %d",result);
}

//int main(){
//	tj_test();
//}



