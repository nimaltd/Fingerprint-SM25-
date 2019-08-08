
#include "Fingerprint.h"
#include "FingerprintConfig.h"
#include <string.h>
#include <stdarg.h>
#if (_FP_USE_FREERTOS==1)
#include "cmsis_os.h"
#endif
#if (_FP_DEBUG>0)
#include <stdio.h>
#endif
//######################################################################################################################
FP_t  FP;
//######################################################################################################################
void  FP_Delay(uint32_t Delay)
{
  #if (_FP_USE_FREERTOS==0)
  HAL_Delay(Delay);
  #else
  osDelay(Delay);
  #endif  
}
//######################################################################################################################
void  FP_RxCallback(void)
{
  if(LL_USART_IsActiveFlag_RXNE(_FP_USART) && LL_USART_IsEnabledIT_RXNE(_FP_USART))
  {
    FP.RxTime = HAL_GetTick();
    if(FP.RxIndex < (sizeof(FP.RxData)))
    {
      FP.RxData[FP.RxIndex] = LL_USART_ReceiveData8(_FP_USART);
      FP.RxIndex++;
    }   
  }
  else  // error
  {
    __NOP();
    
  }
}
//######################################################################################################################
void  FP_Tx(uint8_t Data)
{
  while (!LL_USART_IsActiveFlag_TXE(_FP_USART));
  LL_USART_ClearFlag_TC(_FP_USART); 
  LL_USART_TransmitData8(_FP_USART,Data);
}
//######################################################################################################################
void  FP_TxCmdPacket(uint16_t cmd,uint8_t DataLentgh,...)
{
  uint8_t TxDataArray[24] = {0};
  TxDataArray[0] = 0x55;
  TxDataArray[1] = 0xAA;
  TxDataArray[2] = cmd & 0x00FF;
  TxDataArray[3] = (cmd & 0xFF00) >> 8;
  TxDataArray[4] = DataLentgh;
  TxDataArray[5] = 0;  
  va_list L;
  va_start(L, DataLentgh);
  for (int i=0; i < DataLentgh; i++)
    TxDataArray[i+6] = (uint8_t)va_arg(L, int);
  va_end(L);  
  uint16_t  CheckSum=0;
  for(uint8_t i=0;i<22;i++)
    CheckSum+=TxDataArray[i];
  TxDataArray[22] = CheckSum & 0x00FF; 
  TxDataArray[23] = (CheckSum & 0xFF00) >> 8;   
  memset(FP.RxData,0,sizeof(FP.RxData));
  FP.RxIndex=0;
  #if (_FP_DEBUG==1)
  printf("FP: Send CMD Packet(0x%04X) , Data(",cmd);
  if(DataLentgh==0)
    printf(")\r\n");
  else
  {
    for(uint8_t i=0; i<DataLentgh ; i++)
      printf("0x%02X,",TxDataArray[i+6]);
    printf(")\r\n");
  }
  #endif
  for(uint8_t i=0;i<24;i++)
    FP_Tx(TxDataArray[i]);  
}
//######################################################################################################################
bool  FP_RxPacket(uint16_t Timeout_ms)
{
  uint32_t  StartTime = HAL_GetTick();
  #if (_FP_DEBUG==1)
  uint8_t ErrorCode=0;
  #endif
  while(HAL_GetTick()-StartTime < Timeout_ms)
  {
    if((FP.RxIndex>23) && (HAL_GetTick()-FP.RxTime > 5))  // got new packet
    {
      uint16_t CheckSum=0;
      if((FP.RxData[0] == 0xAA) && (FP.RxData[1] == 0x55)) // Response command packet
      {
        for(uint16_t i=0;i<FP.RxIndex-2;i++)
          CheckSum+=FP.RxData[i];
        if((FP.RxData[FP.RxIndex-2]!=(CheckSum&0x00FF))||(FP.RxData[FP.RxIndex-1]!=(CheckSum&0xFF00)>>8))
        {
          #if (_FP_DEBUG==1)
          ErrorCode=1;
          #endif
          break;
        }
        FP.ResponseCode = FP.RxData[2] | (FP.RxData[3]<<8);
        FP.ResponseCommandLen = FP.RxData[4] | (FP.RxData[5]<<8);
        FP.ResultCode = FP.RxData[6] | (FP.RxData[7]<<8);
        memcpy(FP.ResponseCommand,&FP.RxData[8],14);
        if(FP.ResultCode > 0)
        {
          #if (_FP_DEBUG==1)
          ErrorCode=2;
          #endif
          break;        
        }
      }
      else if((FP.RxData[0] == 0xA5) && (FP.RxData[1] == 0x5A)) // Response data packet
      {
        
        
      }      
      memset(FP.RxData,0,sizeof(FP.RxData));
      FP.RxIndex=0;
      #if (_FP_DEBUG==1)
      printf("FP: Response Packet %d ms.\r\n",HAL_GetTick()-StartTime);
      printf("FP: Response Packet Data(");
      for(uint8_t i=0; i<FP.RxData[4] ; i++)
        printf("0x%02X,",FP.RxData[i+8]);
      printf(")\r\n");
      #endif
      return true;
    }
    FP_Delay(1);
  }  
  #if (_FP_DEBUG==1)
  if(ErrorCode==1)
    printf("FP: Response Packet CheckSum Error!\r\n");
  else if(ErrorCode==2)
  {
    printf("FP: Response Packet Error! (0x%02X%02X)\r\n",FP.RxData[9],FP.RxData[8]);
  }
  else
    printf("FP: Response Packet Timeout after %d ms!\r\n",Timeout_ms);
  #endif
  memset(FP.RxData,0,sizeof(FP.RxData));
  *FP.RxData=0;
  return false;  
}
//######################################################################################################################
//######################################################################################################################
//######################################################################################################################
bool  FP_Init(void)
{
  while(HAL_GetTick()<500)
    FP_Delay(1);
  #if (_FP_DEBUG>0)
  printf("\r\nFP: Begin\r\n");
  #endif
  memset(&FP,0,sizeof(FP));
  LL_USART_EnableIT_RXNE(_FP_USART);
  LL_USART_EnableIT_ERROR(_FP_USART);
  FP_Cancel();
  if(FP_GetFingerTimeOut() != 5)
    FP_SetFingerTimeOut(5);
  return FP_TestConnection();
}
//######################################################################################################################
uint16_t  FP_Identify(void)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;
  #if (_FP_DEBUG==2)
  printf("FP: FP_Identify()\r\n"); 
  #endif
  FP_TxCmdPacket(0x0102,0);  
  if(FP_RxPacket(5000))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      goto next;
    }
    else
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif
      return 0;
    }
    next:
    if(FP_RxPacket(1000))
    {
      if(FP.ResultCode==0)
      {
        FP.Busy=false;
        #if (_FP_DEBUG==2)
        printf("FP: Found %d\r\n",(FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8)));
        #endif
        return (FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8));
      }
      else
      {
        #if (_FP_DEBUG==2)
        printf("FP: ERROR\r\n");
        #endif
        FP.Busy=false;
        return 0;
      }
    }
  }
  else
  {
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif
    FP.Busy=false;
    return 0;   
  }
  #if (_FP_DEBUG==2)
  printf("FP: ERROR\r\n");
  #endif
  FP.Busy=false;
  return 0;  
}
//######################################################################################################################
bool  FP_Enroll(uint16_t  TemplateNo)
{
  FP.EnrollBusy=1;
  FP_Cancel();
  while(FP.Busy)
    FP_Delay(1);  
  FP.Busy=true;
  #if (_FP_DEBUG==2)
  printf("FP: Wait for Put finger,FP_Enroll(%d)\r\n",TemplateNo); 
  #endif  
  FP_UserPutFinger();
  while(HAL_GPIO_ReadPin(_FP_DETECT_GPIO,_FP_DETECT_PIN)==GPIO_PIN_RESET)
    FP_Delay(10);
  FP_Delay(100);
  FP_TxCmdPacket(0x0103,2,TemplateNo&0x00FF,(TemplateNo&0xFF00)>>8);  
  uint8_t step=0;
  while(step<3)
  {
    if(FP_RxPacket(5000))
    {
      if(FP.ResultCode==0)
      {
        goto waitForFinger;
      }
    }
    else
    {
      FP.EnrollBusy=0;
      FP.Busy=false;
      FP_Delay(1000);
      FP_Cancel();
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif
      return false;      
    }
    waitForFinger:
    #if (_FP_DEBUG==2)
    printf("FP: Wait for Put finger\r\n");
    #endif
    FP_UserPutFinger();
    if(FP_RxPacket(5000))
    {
      if(FP.ResultCode==0)
      {
        goto again;
      }
    }
    else
    {
      FP.EnrollBusy=0;
      FP.Busy=false;
      FP_Delay(1000);
      FP_Cancel();
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif
      return false;      
    }   
    again:
    FP_UserRelaseFinger();
    #if (_FP_DEBUG==2)
    printf("FP: Step %d Done\r\n",step+1);
    printf("FP: Wait for Release finger\r\n");
    #endif
    step++;    
  }  
  if(FP_RxPacket(10000))
  {
    if(FP.ResultCode==0)
    {
      while(HAL_GPIO_ReadPin(_FP_DETECT_GPIO,_FP_DETECT_PIN)==GPIO_PIN_SET)
        FP_Delay(10);
      FP_Delay(100);
      FP.EnrollBusy=0;
      FP.Busy=false;
      #if (_FP_DEBUG==2)
        printf("FP: FP_Enroll(%d) Done\r\n",TemplateNo); 
      #endif      
      return true;      
    }
  }
  else
  {
    FP.EnrollBusy=0;
    FP.Busy=false;
    FP_Delay(1000);
    FP_Cancel();
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif
    return false;      
  }
  FP.Busy=false;
  FP.EnrollBusy=0;
  FP_Delay(1000);
  FP_Cancel();
  #if (_FP_DEBUG==2)
  printf("FP: ERROR\r\n");
  #endif  
  return false;
}
//######################################################################################################################
bool  FP_ClearTemplate(uint16_t  TemplateNo)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;
  #if (_FP_DEBUG==2)
  printf("FP: FP_ClearTemplate(%d)\r\n",TemplateNo);
  #endif
  FP_TxCmdPacket(0x0105,2,TemplateNo&0x00FF,(TemplateNo&0xFF00)>>8);  
  if(FP_RxPacket(2000))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: Done\r\n");
      #endif
      return true;
    }
    else
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif
      return false;
    }
  }
  else
  {
    FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif
    return false; 
  }    
}
//######################################################################################################################
bool  FP_ClearAllTemplate(void)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;
  #if (_FP_DEBUG==2)
  printf("FP: FP_ClearAllTemplate()\r\n");
  #endif  
  FP_TxCmdPacket(0x0106,0);  
  if(FP_RxPacket(2000))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: Done\r\n");
      #endif
      return true;
    }
    else
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif
      return false;
    }
  }
  else
  {
    FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif    
    return false; 
  }    
}
//######################################################################################################################
uint16_t  FP_GetEmptyID(void)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;
  #if (_FP_DEBUG==2)
  printf("FP: FP_GetEmptyID()\r\n");
  #endif    
  FP_TxCmdPacket(0x0107,0);  
  if(FP_RxPacket(2000))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: Fist EmptyID: %d \r\n",(FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8)));
      #endif    
      return (FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8));
    }
    else
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif    
      return 0;
    }
  }
  else
  {
    FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif    
    return 0;   
  }
}
//######################################################################################################################
bool  FP_SetFingerTimeOut(uint16_t  Timeout)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;
  #if (_FP_DEBUG==2)
  printf("FP: FP_SetFingerTimeOut(%d)\r\n",Timeout);
  #endif 
  FP_TxCmdPacket(0x010E,2,Timeout&0x00FF,(Timeout&0xFF00)>>8);  
  if(FP_RxPacket(2000))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: Done\r\n");
      #endif      
      return true;
    }
    else
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif        
      return false;
    }
  }
  else
  {
    FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif      
    return false;
  }
}
//######################################################################################################################
uint16_t  FP_GetFingerTimeOut(void)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;
  #if (_FP_DEBUG==2)
  printf("FP: FP_GetFingerTimeOut()\r\n");
  #endif   
  FP_TxCmdPacket(0x010F,0);  
  if(FP_RxPacket(2000))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: Finger TimeOut: %d s \r\n",(FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8)));
      #endif      
      return (FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8));
    }
    else
    {
      FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif       
      return 0;
    }
  }
  else
  {
    FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif     
    return 0;  
  }
}
//######################################################################################################################
uint16_t  FP_GetFirmwareVersion(void)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;  
  #if (_FP_DEBUG==2)
  printf("FP: FP_GetFirmwareVersion()\r\n");
  #endif    
  FP_TxCmdPacket(0x0112,0);  
  if(FP_RxPacket(200))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: FirmwareVersion: 0x%04X\r\n",(FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8)));
      #endif  
      return (FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8));
    }
    else
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif        
      return 0;
    }
  }
  else
  {
    FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif      
    return 0;  
  }
}
//######################################################################################################################
bool  FP_EnterStadbyMode(void)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;  
  #if (_FP_DEBUG==2)
  printf("FP: FP_EnterStadbyMode()\r\n");
  #endif   
  FP_TxCmdPacket(0x0117,0);  
  if(FP_RxPacket(200))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: Done\r\n");
      #endif        
      return true;
    }
    else
    {
      FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif        
      return false;
    }
  }
  else
  {
    FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif      
    return false; 
  }    
}
//######################################################################################################################
uint16_t  FP_GetEnrollCount(void)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;  
  #if (_FP_DEBUG==2)
  printf("FP: FP_GetEnrollCount()\r\n");
  #endif  
  FP_TxCmdPacket(0x0128,0);  
  if(FP_RxPacket(200))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: Enroll Count: %d\r\n",(FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8)));
      #endif
      return (FP.ResponseCommand[0]|(FP.ResponseCommand[1]<<8));
    }
    else
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif            
      return 0;
    }
  }
  else
  {
    FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif          
    return 0;  
  }
}
//######################################################################################################################
bool  FP_TestConnection(void)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;  
  #if (_FP_DEBUG==2)
  printf("FP: FP_TestConnection()\r\n");
  #endif    
  FP_TxCmdPacket(0x0150,0);  
  if(FP_RxPacket(100))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: Done\r\n");
      #endif 
      return true;
    }
    else
    {
      FP.Busy=false;
      #if (_FP_DEBUG==2)
      printf("FP: ERROR\r\n");
      #endif         
      return false;
    }
  }
  else
  {
    FP.Busy=false;
    #if (_FP_DEBUG==2)
    printf("FP: ERROR\r\n");
    #endif       
    return false; 
  }    
}
//######################################################################################################################
bool  FP_Cancel(void)
{
  while(FP.Busy)
    FP_Delay(1);
  FP.Busy=true;  
  FP_TxCmdPacket(0x0130,0);  
  if(FP_RxPacket(200))
  {
    if(FP.ResultCode==0)
    {
      FP.Busy=false;
      return true;
    }
    else
    {
      FP.Busy=false;
      return false;
    }
  }
  else
  {
    FP.Busy=false;
    return false;  
  }
}
//######################################################################################################################
void  FP_Loop(void)
{
  static uint32_t FP_Loop_Time=0;
  static uint8_t  FP_DetectBit=0;
  if(FP.EnrollBusy==1)
    return;
  if((HAL_GPIO_ReadPin(_FP_DETECT_GPIO,_FP_DETECT_PIN)==GPIO_PIN_SET) && (FP_DetectBit==0))
  {
    if(HAL_GetTick() - FP_Loop_Time > 500)
    {
      FP_Loop_Time = HAL_GetTick();
      FP_DetectBit=1;
      uint16_t f = FP_Identify();
      if(f>0)
        FP_UserDetect(f);    
    }  
  }
  if((HAL_GPIO_ReadPin(_FP_DETECT_GPIO,_FP_DETECT_PIN)==GPIO_PIN_RESET) && (FP_DetectBit==1))
  {
    FP_DetectBit=0;    
  }
}
//######################################################################################################################
