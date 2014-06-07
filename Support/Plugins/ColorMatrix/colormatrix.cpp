/*
OFX ColorMatrix Example plugin, a plugin that illustrates the use of the OFX Support library.

Copyright (C) 2007 The Open Effects Association Ltd
Author Marc-Antoine Arnaud arnaud.marcantoine@gmail.com

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
* Neither the name The Open Effects Association Ltd, nor the names of its 
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The Open Effects Association Ltd
1 Wardour St
London W1D 6PA
England


*/

#ifdef _WINDOWS
#include <windows.h>
#endif

#ifdef __APPLE__
#include <AGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <cstdio>
#include <cstring>
#include "ofxsImageEffect.h"
#include "ofxsMultiThread.h"

#include "../include/ofxsProcessing.H"


// Base class for the RGBA and the Alpha processor
class ColorMatrixBase : public OFX::ImageProcessor {
protected :
  OFX::Image *_srcImg;
  float matrix44[16];
public :
  /** @brief no arg ctor */
  ColorMatrixBase(OFX::ImageEffect &instance)
    : OFX::ImageProcessor(instance)
    , _srcImg(0)
  {
  }

  /** @brief set the src image */
  void setSrcImg(OFX::Image *v) {_srcImg = v;}

  /** @brief set matrix coefficients */
  void setMatricCoefficients(
    float rr, float rg, float rb, float ra,
    float gr, float gg, float gb, float ga,
    float br, float bg, float bb, float ba,
    float ar, float ag, float ab, float aa
    ){
    matrix44[0] = rr;
    matrix44[1] = rg;
    matrix44[2] = rb;
    matrix44[3] = ra;
    matrix44[4] = gr;
    matrix44[5] = gg;
    matrix44[6] = gb;
    matrix44[7] = ga;
    matrix44[8] = br;
    matrix44[9] = bg;
    matrix44[10] = bb;
    matrix44[11] = ba;
    matrix44[12] = ar;
    matrix44[13] = ag;
    matrix44[14] = ab;
    matrix44[15] = aa;
  }  
};

// template to do the RGB/RGBA processing
template <class Pixel, int nComponents, int max>
class ImageColorMatrix : public ColorMatrixBase {
public :
  // ctor
  ImageColorMatrix(OFX::ImageEffect &instance) 
    : ColorMatrixBase(instance)
  {}

  // and do some processing
  void multiThreadProcessImages(OfxRectI procWindow)
  {
    for(int y = procWindow.y1; y < procWindow.y2; y++) {
      if(_effect.abort()) break;

      Pixel *dstPix = (Pixel *) _dstImg->getPixelAddress(procWindow.x1, y);

      for(int x = procWindow.x1; x < procWindow.x2; x++) {

        Pixel *srcPix = (Pixel *)  (_srcImg ? _srcImg->getPixelAddress(x, y) : 0);

        // do we have a source image to scale up
        if(srcPix) {
          float inR = srcPix[0];
          float inG = srcPix[1];
          float inB = srcPix[2];
          float inA = (nComponents == 4) ? srcPix[3] : 0.0;
          for( int c = 0; c < nComponents; ++c ){
            dstPix[c] = matrix44[4*c] * inR + matrix44[4*c+1] * inG + matrix44[4*c+2] * inB + matrix44[4*c+3] * inA;
          }
        }
        else {
          // no src pixel here, be black and transparent
          memset(dstPix ,0, nComponents);
        }

        // increment the dst pixel
        dstPix += nComponents;
      }
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
/** @brief The plugin that does our work */
class ColorMatrixPlugin : public OFX::ImageEffect {
protected :
  // do not need to delete these, the ImageEffect is managing them for us
  OFX::Clip *dstClip_;
  OFX::Clip *srcClip_;

  OFX::RGBAParam *outputRed_;
  OFX::RGBAParam *outputGreen_;
  OFX::RGBAParam *outputBlue_;
  OFX::RGBAParam *outputAlpha_;

public :
  /** @brief ctor */
  ColorMatrixPlugin(OfxImageEffectHandle handle)
    : ImageEffect(handle)
    , dstClip_(0)
    , srcClip_(0)
    , outputRed_(0)
    , outputGreen_(0)
    , outputBlue_(0)
    , outputAlpha_(0)
  {
    dstClip_ = fetchClip("Output");
    srcClip_ = fetchClip("Source");
    outputRed_   = fetchRGBAParam("OutputRed");
    outputGreen_ = fetchRGBAParam("OutputGreen");
    outputBlue_  = fetchRGBAParam("OutputBlue");
    outputAlpha_ = fetchRGBAParam("OutputAlpha");
  }

  /* Override the render */
  virtual void render(const OFX::RenderArguments &args);

  /* set up and run a processor */
  void setupAndProcess(ColorMatrixBase &, const OFX::RenderArguments &args);
};


////////////////////////////////////////////////////////////////////////////////
/** @brief render for the filter */

////////////////////////////////////////////////////////////////////////////////
// basic plugin render function, just a skelington to instantiate templates from


/* set up and run a processor */
void
ColorMatrixPlugin::setupAndProcess(ColorMatrixBase &processor, const OFX::RenderArguments &args)
{
  // get a dst image
  std::auto_ptr<OFX::Image> dst(dstClip_->fetchImage(args.time));
  OFX::BitDepthEnum dstBitDepth       = dst->getPixelDepth();
  OFX::PixelComponentEnum dstComponents  = dst->getPixelComponents();

  // fetch main input image
  std::auto_ptr<OFX::Image> src(srcClip_->fetchImage(args.time));

  // make sure bit depths are sane
  if(src.get()) {
    OFX::BitDepthEnum    srcBitDepth      = src->getPixelDepth();
    OFX::PixelComponentEnum srcComponents = src->getPixelComponents();

    // see if they have the same depths and bytes and all
    if(srcBitDepth != dstBitDepth || srcComponents != dstComponents)
      throw int(1); // HACK!! need to throw an sensible exception here!
  }

  // set the images
  processor.setDstImg(dst.get());
  processor.setSrcImg(src.get());

  double rr = 0.0;
  double rg = 0.0;
  double rb = 0.0;
  double ra = 0.0;
  double gr = 0.0;
  double gg = 0.0;
  double gb = 0.0;
  double ga = 0.0;
  double br = 0.0;
  double bg = 0.0;
  double bb = 0.0;
  double ba = 0.0;
  double ar = 0.0;
  double ag = 0.0;
  double ab = 0.0;
  double aa = 0.0;
  
  outputRed_->getValueAtTime(args.time, rr, rg, rb, ra);
  outputGreen_->getValueAtTime(args.time, gr, gg, gb, ga);
  outputBlue_->getValueAtTime(args.time, br, bg, bb, ba);
  outputAlpha_->getValueAtTime(args.time, ar, ag, ab, aa);

  processor.setMatricCoefficients(
    rr, rg, rb, ra,
    gr, gg, gb, ga,
    br, bg, bb, ba,
    ar, ag, ab, aa
    );

  // set the render window
  processor.setRenderWindow(args.renderWindow);

  // Call the base class process member, this will call the derived templated process code
  processor.process();
}

// the overridden render function
void
ColorMatrixPlugin::render(const OFX::RenderArguments &args)
{
  // instantiate the render code based on the pixel depth of the dst clip
  OFX::BitDepthEnum       dstBitDepth    = dstClip_->getPixelDepth();
  OFX::PixelComponentEnum dstComponents  = dstClip_->getPixelComponents();

  // do the rendering
  if(dstComponents == OFX::ePixelComponentRGBA) {
    switch(dstBitDepth) {
      case OFX::eBitDepthUByte : {      
        ImageColorMatrix<unsigned char, 4, 255> fred(*this);
        setupAndProcess(fred, args);
        break;
      }

      case OFX::eBitDepthUShort : {
        ImageColorMatrix<unsigned short, 4, 65535> fred(*this);
        setupAndProcess(fred, args);
        break;
      }                          

      case OFX::eBitDepthFloat : {
        ImageColorMatrix<float, 4, 1> fred(*this);
        setupAndProcess(fred, args);
        break;
      }
    }
  } else { // RGB Components
    switch(dstBitDepth) {
      case OFX::eBitDepthUByte : {
        ImageColorMatrix<unsigned char, 3, 255> fred(*this);
        setupAndProcess(fred, args);
        break;
      }

      case OFX::eBitDepthUShort : {
        ImageColorMatrix<unsigned short, 3, 65536> fred(*this);
        setupAndProcess(fred, args);
        break;
      }                          

      case OFX::eBitDepthFloat : {
        ImageColorMatrix<float, 3, 1> fred(*this);
        setupAndProcess(fred, args);
        break;
      }                          
    }
  } 
}

mDeclarePluginFactory(ColorMatrixPluginFactory, {}, {});

using namespace OFX;
void ColorMatrixPluginFactory::describe(OFX::ImageEffectDescriptor &desc)
{
  // basic labels
  desc.setLabels("ColorMatrix", "ColorMatrix", "ColorMatrix");
  desc.setPluginGrouping("OFX");

  // add the supported contexts, only filter at the moment
  desc.addSupportedContext(eContextFilter);

  // add supported pixel depths
  desc.addSupportedBitDepth(eBitDepthUByte);
  desc.addSupportedBitDepth(eBitDepthUShort);
  desc.addSupportedBitDepth(eBitDepthFloat);

  // set a few flags
  desc.setSingleInstance(false);
  desc.setHostFrameThreading(false);
  desc.setSupportsMultiResolution(true);
  desc.setSupportsTiles(true);
  desc.setTemporalClipAccess(false);
  desc.setRenderTwiceAlways(false);
  desc.setSupportsMultipleClipPARs(false);
}

void ColorMatrixPluginFactory::describeInContext(OFX::ImageEffectDescriptor &desc, OFX::ContextEnum context)
{
  // Source clip only in the filter context
  // create the mandated source clip
  ClipDescriptor *srcClip = desc.defineClip("Source");
  srcClip->addSupportedComponent(ePixelComponentRGBA);
  srcClip->addSupportedComponent(ePixelComponentRGB);
  srcClip->setTemporalClipAccess(false);
  srcClip->setSupportsTiles(true);
  srcClip->setIsMask(false);

  // create the mandated output clip
  ClipDescriptor *dstClip = desc.defineClip("Output");
  dstClip->addSupportedComponent(ePixelComponentRGBA);
  dstClip->addSupportedComponent(ePixelComponentRGB);
  dstClip->setSupportsTiles(true);

  RGBAParamDescriptor *paramRed = desc.defineRGBAParam("OutputRed");
  paramRed->setLabels("output_red", "output_red", "output_red");
  paramRed->setScriptName("output_red");
  paramRed->setHint("values for red output component.");
  paramRed->setDefault(1.0, 0.0, 0.0, 0.0);
  paramRed->setAnimates(true); // can animate

  RGBAParamDescriptor *paramGreen = desc.defineRGBAParam("OutputGreen");
  paramGreen->setLabels("output_green", "output_green", "output_green");
  paramGreen->setScriptName("output_green");
  paramGreen->setHint("values for green output component.");
  paramGreen->setDefault(0.0, 1.0, 0.0, 0.0);
  paramGreen->setAnimates(true); // can animate

  RGBAParamDescriptor *paramBlue = desc.defineRGBAParam("OutputBlue");
  paramBlue->setLabels("output_blue", "output_blue", "output_blue");
  paramBlue->setScriptName("output_blue");
  paramBlue->setHint("values for blue output component.");
  paramBlue->setDefault(0.0, 0.0, 1.0, 0.0);
  paramBlue->setAnimates(true); // can animate

  RGBAParamDescriptor *paramAlpha = desc.defineRGBAParam("OutputAlpha");
  paramAlpha->setLabels("output_aplha", "output_aplha", "output_aplha");
  paramAlpha->setScriptName("output_aplha");
  paramAlpha->setHint("values for alpha output component.");
  paramAlpha->setDefault(0.0, 0.0, 0.0, 1.0);
  paramAlpha->setAnimates(true); // can animate

  PageParamDescriptor *page = desc.definePageParam("Controls");
  page->addChild(*paramRed);
  page->addChild(*paramGreen);
  page->addChild(*paramBlue);
  page->addChild(*paramAlpha);
}

OFX::ImageEffect* ColorMatrixPluginFactory::createInstance(OfxImageEffectHandle handle, OFX::ContextEnum context)
{
  return new ColorMatrixPlugin(handle);
}

namespace OFX 
{
  namespace Plugin 
  {  
    void getPluginIDs(OFX::PluginFactoryArray &ids)
    {
      static ColorMatrixPluginFactory p("net.sf.openfx:colormatrix", 1, 0);
      ids.push_back(&p);
    }
  }
}
