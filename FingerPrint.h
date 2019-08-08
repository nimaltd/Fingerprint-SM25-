#ifndef _FINGERPRINT_H
#define _FINGERPRINT_H
//########################################################################################
#include <stdbool.h>
#include "usart.h"
//########################################################################################
typedef struct
{
  uint8_t   RxData[24];
  uint16_t  RxIndex;
  uint32_t  RxTime;
  
  uint8_t   ResponseCommand[14];
  uint16_t  ResponseCommandLen;
  uint16_t  ResponseCode;
  uint16_t  ResultCode;
  bool      Busy;
  bool      EnrollBusy;
  
}FP_t;

//########################################################################################
void      FP_RxCallback(void);
void      FP_Loop(void);
void      FP_UserDetect(uint16_t TemplateNo);
void      FP_UserRelaseFinger(void);
void      FP_UserPutFinger(void);

bool      FP_Init(void);
bool      FP_TestConnection(void);
bool      FP_Cancel(void);
uint16_t  FP_Identify(void);
bool      FP_Enroll(uint16_t TemplateNo);
bool      FP_ClearTemplate(uint16_t TemplateNo);
bool      FP_ClearAllTemplate(void);
uint16_t  FP_GetEmptyID(void);
bool      FP_SetFingerTimeOut(uint16_t Timeout);
uint16_t  FP_GetFingerTimeOut(void);
uint16_t  FP_GetFirmwareVersion(void);
bool      FP_EnterStadbyMode(void);
uint16_t  FP_GetEnrollCount(void);
//########################################################################################
#endif
