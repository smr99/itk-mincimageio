/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMINC2ImageIO.h,v $
  Language:  C++
  Date:      $Date: 2008-12-28 08:08:48 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
/**
 *         The specification for this file format is taken from the
 *         web site http://www.bic.mni.mcgill.ca/software/minc
 * \author Leila Baghdadi
 *         Mouse Imaging Centre, Toronto, Canada 2005.
 */


#ifndef __itkMINCImageIO_h
#define __itkMINCImageIO_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "itkImageIOBase.h"

extern "C" {
#include <minc2.h>
}


namespace itk
{

/** \class MINCImageIO
 *
 * \author Leila Baghdadi
 * \brief Class that defines how to read MINC2 file format. Note,like
 * ITK, MINC2 is N dimensional and dimensions can be submitted in any 
 * arbitrary order. Here we make sure the dimensions are ordered as
 * xspace, yspace, zspace, time and vector_dimension and so on or
 * xfrequencey, yfrequency, zfrequency, tfrequency and
 * vector_dimension and so on
 * NOTE** This class only reads the regularly sampled dimensions as I
 * am not sure how to deal with "iregularly sampled" dimensions yet!
 * \ingroup IOFilters
 *
 */
class ITK_EXPORT MINCImageIO : public ImageIOBase
{
public:
  /** Standard class typedefs. */
  typedef MINCImageIO             Self;
  typedef ImageIOBase             Superclass;
  typedef SmartPointer<Self>      Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MINCImageIO, ImageIOBase);

  /*-------- This part of the interface deals with reading data. ------ */

  virtual bool CanReadFile(const char*);
  virtual void ReadImageInformation();
  virtual void Read(void* buffer);

  /*-------- This part of the interfaces deals with writing data. ----- */

  virtual bool CanWriteFile(const char*);
  virtual void WriteImageInformation();
  virtual void Write(const void* buffer);

  /*-------- This part of the interfaces deals with other stuff. ----- */

  virtual bool SupportsDimension( unsigned long dim )
  {
    return dim >= 2;
  }
  
protected:
  MINCImageIO();
  ~MINCImageIO();
  
  void PrintSelf(std::ostream& os, Indent indent) const;

private:
  MINCImageIO(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  // Set pixel type, component type, number of components from the
  // file.  
  // Calls: SetPixelType(), SetComponentType(), SetNumberOfComponents().
  void ReadPixelInformation();

  // Set image shape information (dimensions, size of each dimension)
  // from the file.
  // Calls: SetNumberOfDimensions(), SetDimensions().
  void ReadShapeInformation();

  // Set image-to-world transformation (origin, spacing, direction
  // cosines) from the file.
  // Calls: SetOrigin(), SetSpacing(), SetDirection()
  void ReadImageToWorldInformation();

  void SetDirectionFromCosines( unsigned int i, double cosines[3] );

  // Close cached MINC file handle, if open.
  void CloseVolume();

  // MINC file handle, cached between calls to ReadImageInformation()
  // and Read().  The flag m_VolumeValid indicates whether the handle
  // is valid.
  mihandle_t m_Volume;
  bool m_VolumeValid;

  // Set as a side-effect of ReadShapeInformation().
  midimhandle_t* m_VolumeDimension;
};

} // end namespace itk

#endif // __itkMINCImageIO_h
