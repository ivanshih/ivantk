/*=========================================================================

Image-based Vascular Analysis Toolkit (IVAN)

Copyright (c) 2012, Iván Macía Oliver
Vicomtech Foundation, San Sebastián - Donostia (Spain)
University of the Basque Country, San Sebastián - Donostia (Spain)

All rights reserved

See LICENSE file for license details

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

==========================================================================*/
// File: ivanCircularBarTubeGenerator.h
// Author: Iv�n Mac�a (imacia@vicomtech.org)
// Description: creates a Gaussian tube of the given size, stddev and peak intensity value
// Date: 2010/09/17


#ifndef __ivanCircularBarTubeGenerator_h_
#define __ivanCircularBarTubeGenerator_h_

#include "ivanCircularBarSectionGenerator.h"

#include "itkImageRegionIterator.h"

namespace ivan
{

template <class TPixel>
class CircularBarTubeGenerator
{
public:

  typedef TPixel                        PixelType;
  typedef itk::Image<PixelType,3>       ImageType;
  typedef typename ImageType::Pointer   ImagePointer;
  
public:

  CircularBarTubeGenerator();
  ~CircularBarTubeGenerator() {};
  
  void SetSectionImageSize( unsigned long size )
    { m_SectionImageSize = size; }
  unsigned long GetSectionImageSize() const
    { return m_SectionImageSize; }
  
  void SetHeight( unsigned long height )
    { m_Height = height; }
  unsigned long GetHeight() const
    { return m_Height; }
    
  void SetImageSpacing( double spacing )
    { m_ImageSpacing = spacing; }
  double GetImageSpacing() const
    { return m_ImageSpacing; }

  /** Set the tube radius. */
  void SetTubeRadius( double radius )
    { m_TubeRadius = radius; }
  double GetRadius() const
    { return m_TubeRadius; }

  /** Add an offset in the Z-direction so we don't cut the tube at the Z boundaries. */
  void SetZOffset( double offset )
    { m_ZOffset = offset; }
  double GetZOffset() const
    { return m_ZOffset; }

  void SetMaxValue( PixelType value )
    { m_MaxValue = value; }
  PixelType GetMaxValue() const
    { return m_MaxValue; }

  ImagePointer Create();

protected: 
  
  /** Image size for the section (same for width and depth). */
  unsigned long    m_SectionImageSize;
  
  /** Tube length in pixels. */
  unsigned long    m_Height;
  
  /** Image spacing. */
  double           m_ImageSpacing;
  
  /** Radius of the tube section. WARNING: do not rename this to m_Radius since
    * it will conflict with subclasses that need to define more than one radius. */
  double           m_TubeRadius;
  
  /** Offset used so we don't cut the tube at the Z boundaries. */
  double           m_ZOffset;

  /** Maximum intensity value. */
  PixelType        m_MaxValue;
};


template <class TPixel>
CircularBarTubeGenerator<TPixel>::CircularBarTubeGenerator() :
  m_SectionImageSize( 50 ),
  m_Height( 100 ),
  m_ImageSpacing( 1.0 ),  
  m_TubeRadius( 2.0 ),
  m_ZOffset( 0.0 ),
  m_MaxValue( 255.0 )
{
  
}


template <class TPixel>
typename CircularBarTubeGenerator<TPixel>::ImagePointer
CircularBarTubeGenerator<TPixel>::Create()
{
  ImagePointer tubeImage = ImageType::New();
  
  typename ImageType::SpacingType spacing;
  spacing.Fill( this->m_ImageSpacing );
  
  tubeImage->SetSpacing( spacing );
  
  typename ImageType::PointType origin;
  origin.Fill( 0.0 );
  
  tubeImage->SetOrigin( origin );
  
  double totalHeight = this->m_Height + 2.0 * this->m_ZOffset;
  
  typename ImageType::RegionType::SizeType size;
  size[0] = this->m_SectionImageSize;
  size[1] = this->m_SectionImageSize;
  size[2] = (unsigned long) floor( ( totalHeight / this->m_ImageSpacing ) + 0.5 );
  
  typename ImageType::RegionType::IndexType index;
  index.Fill(0);
  
  typename ImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( index );
  
  tubeImage->SetRegions( region );
  
  tubeImage->Allocate();
  tubeImage->FillBuffer(0);
  
  // Crate Gaussian section
  
  typedef CircularBarSectionGenerator<TPixel>      SectionGeneratorType;
  typedef typename SectionGeneratorType::ImageType  SectionImageType;
    
  SectionGeneratorType sectionGenerator;
  sectionGenerator.SetImageSize( this->m_SectionImageSize );
  sectionGenerator.SetRadius( this->m_TubeRadius );
  sectionGenerator.SetMaxValue( this->m_MaxValue );
  
  typename SectionImageType::Pointer sectionImage = sectionGenerator.Create();
  
  typedef itk::ImageRegionConstIterator<SectionImageType>  ConstIteratorType;
  typedef itk::ImageRegionIterator<ImageType>              IteratorType;
  
  ConstIteratorType inputIt( sectionImage, sectionImage->GetRequestedRegion() );
  IteratorType outputIt;
  
  size[2] = 1; // copy a single slice each time
  region.SetSize( size );
  
  for( unsigned long z=0; z < m_Height; ++z )
  {
    index[2] = z; // current slice
    region.SetIndex( index );
    
    outputIt = IteratorType( tubeImage, region );
    
    inputIt.GoToBegin();
    outputIt.GoToBegin();
    
    while( !inputIt.IsAtEnd() )
    {
      outputIt.Set( inputIt.Get() );
      ++inputIt;
      ++outputIt;
    }
  }
  
  return tubeImage;    
}

} // end namespace ivan

#endif // __ivanCircularBarTubeGenerator_h_

