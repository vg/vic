/*
 *  Copyright (c) 1996 John Brezak
 *  Copyright (c) 1996 Isidor Kouvelas (University College London)
 *  Portions Copyright (c) 2004 EarthLink, Inc.
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// grabber-win32DS.h

//warning C4996: 'strcpy': This function or variable may be unsafe. Consider using strcpy_s instead. 
// To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <dshow.h>   // DirectShow
#include <amstream.h>   // DirectShow

#pragma include_alias("dxtrans.h", "qedit.h")
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
// dxtrans.h is missing with some DirectX SDKs and VisualStudio versions - broken M$ setup?!
// the above #pragma and #define's make sure it's contents is not needed
#include <qedit.h>   // DirectShow

#include "crossbar.h"

#define NUM_DEVS 20   // max number of capture devices we'll support
#define NUM_PORTS 20  // max number of ports on a capture device

//#define showErrorMessage(x)   ShowErrorMessage(x, __LINE__, __FILE__)

//extern void ShowErrorMessage(HRESULT, int, char* );

#define CF_422 0
#define CF_420 1
#define CF_CIF 2

static const int D1_BASE_WIDTH  = 720;
static const int D1_BASE_HEIGHT = 480;
static const int NTSC_BASE_WIDTH  = 640;
static const int NTSC_BASE_HEIGHT = 480;
static const int PAL_BASE_WIDTH   = 768;
static const int PAL_BASE_HEIGHT  = 568;
static const int CIF_BASE_WIDTH   = 704;
static const int CIF_BASE_HEIGHT  = 576;

//#########################################################################

/*
class Crossbar {
   public:
      Crossbar(IAMCrossbar *);      
      IAMCrossbar *getXBar();

      Crossbar *next;
   private:
      IAMCrossbar  *xbar;      
};
*/

//#########################################################################

class Callback;

struct Port {
       int id;
       char name[64];
};

class DirectShowGrabber : public Grabber {
   public:
      DirectShowGrabber(IBaseFilter *, const char * cformat, const char * nick = 0);
      ~DirectShowGrabber();
      virtual int  command(int argc, const char*const* argv);

	  inline void  converter(Converter* v) {
		  converter_ = v;
	  }

	  void         capture(BYTE *, long);

	  bool		   hasDV_SD(){
	      return (have_DVSD_);
	  }

	  int		   maxWidth(){
	      return max_width_;
	  }

	  int		   maxHeight(){
		  return max_height_;
	  }

	  int		   minWidth(){
	      return min_width_;
	  }

	  int		   minHeight(){
		  return min_height_;
	  }

	  Port **	   getInputPorts(){
		  return inputPorts;
	  }

      int          capturing_;
      HANDLE       cb_mutex_;
   protected:
      virtual void start();
      virtual void stop();
      virtual void fps(int);
      virtual void setsize();
      virtual int  grab();
      void         setport(const char *port);
      int          getCaptureCapabilities(int preferred_max_width=1024);
      virtual void setCaptureOutputFormat();

      int          useconfig_;
      int          basewidth_;
      int          baseheight_;
      u_int        max_fps_;
      int          max_width_;
      int          max_height_;
      int          min_width_;
      int          min_height_;
      int          width_;
      int          height_;
      int          cformat_;

      Port *       inputPorts[NUM_PORTS];
      int          numInputPorts;
      int          initializedPorts;
      int          compositePortNum;
      int          svideoPortNum;

      bool         have_I420_;  // YUV 4:2:0 planar
      bool         have_UYVY_;  // YUV 4:2:2 packed
      bool         have_YUY2_;  // as for UYVY but with different component ordering
      bool         have_RGB24_; // RGB 24 bit
      bool         have_DVSD_;  // DV standard definition

      u_int        decimate_;    // set in this::command via small/normal/large in vic UI; msp
      BYTE         *last_frame_;
      Converter    *converter_;

   private:
      IBaseFilter*           pFilter_;
      IBaseFilter*           pCaptureFilter_;
      ISampleGrabber*        pSampleGrabber_;
      IBaseFilter*           pGrabberBaseFilter_;
      IIPDVDec*              pDVVideoDecoder_;
      IBaseFilter*           pDVDecoderBaseFilter_;
      IBaseFilter*           pNullRenderer_;
      IBaseFilter*           pNullBaseFilter_;
      IGraphBuilder*         pGraph_;
      ICaptureGraphBuilder2* pBuild_;
      IMediaControl*         pMediaControl_;
      DWORD                  dwRegister_;
      AM_MEDIA_TYPE          mt_;
      Callback               *callback_;

      IAMCrossbar            *pXBar_;
      Crossbar               *crossbar_;
      Crossbar               *crossbarCursor_;
      char                   input_port_[20];
      bool                   findCrossbar(IBaseFilter *);
      void                   addCrossbar(IAMCrossbar *);
      void                   routeCrossbar();

};

//#########################################################################

class Callback : public ISampleGrabberCB {
   private:
      volatile long m_cRef;
   public:
      Callback() {}      

      DirectShowGrabber *grabber;

      // IUnknown methods
      STDMETHODIMP_(ULONG) AddRef() {
         return InterlockedIncrement(&m_cRef);
      }

      STDMETHODIMP_(ULONG) Release() {
         long lCount = InterlockedDecrement(&m_cRef);
         if (lCount == 0) {
            delete this;
         }
         // Return the temporary variable, not the member
         // variable, for thread safety.
         return (ULONG)lCount;
      }

      STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);

      // our callbacks
      STDMETHODIMP SampleCB(double sampleTime, IMediaSample *pSample);
      STDMETHODIMP BufferCB(double sampleTime, BYTE *pBuffer, long bufferLen);
};

//#########################################################################

class DirectShowDevice : public InputDevice {
   public:
      DirectShowDevice(char *, IBaseFilter *);
	  ~DirectShowDevice();
      virtual int command(int argc, const char* const* argv);     

   protected:
      IBaseFilter*           pDirectShowFilter_;

      //IBaseFilter       *directShowFilter_;
      DirectShowGrabber *directShowGrabber_;   
	  char *attri_;
};

//#########################################################################

class DirectShowScanner {
   public:      
      DirectShowScanner();
      ~DirectShowScanner();
   protected:
      DirectShowDevice *devs_[NUM_DEVS];
      IMoniker*     pMoniker_;

};
