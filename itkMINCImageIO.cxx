#include "itkMINCImageIO.h"

#include <cstring>
#include <cassert>



namespace itk {


namespace {

itk::ImageIOBase::IOComponentType ConvertDataTypeToITK( const mitype_t& mincType )
{
  switch( mincType )
    {
    case MI_TYPE_BYTE:
      return( itk::ImageIOBase::CHAR );
    case MI_TYPE_SHORT:
    case MI_TYPE_SCOMPLEX:
      return( itk::ImageIOBase::SHORT );
    case MI_TYPE_INT:
    case MI_TYPE_ICOMPLEX:
      return( itk::ImageIOBase::INT );
    case MI_TYPE_FLOAT:
    case MI_TYPE_FCOMPLEX:
      return( itk::ImageIOBase::FLOAT );
    case MI_TYPE_DOUBLE:
    case MI_TYPE_DCOMPLEX:
      return( itk::ImageIOBase::DOUBLE );
    case MI_TYPE_UBYTE:
      return( itk::ImageIOBase::UCHAR );
    case MI_TYPE_USHORT:
      return( itk::ImageIOBase::USHORT );
    case MI_TYPE_UINT:
      return( itk::ImageIOBase::UINT );
    default:
      itkGenericOutputMacro(<< "unhandled MINC data type: " << mincType);
    }
  return itk::ImageIOBase::UNKNOWNCOMPONENTTYPE;
}

mitype_t ConvertScalarDataTypeToMINC( const itk::ImageIOBase::IOComponentType& componentType )
{
  switch( componentType )
    {
    case itk::ImageIOBase::UCHAR:
      return MI_TYPE_UBYTE;
    case itk::ImageIOBase::CHAR:
      return MI_TYPE_BYTE;
    case itk::ImageIOBase::USHORT:
      return MI_TYPE_USHORT;
    case itk::ImageIOBase::SHORT:
      return MI_TYPE_SHORT;
    case itk::ImageIOBase::UINT:
      return MI_TYPE_UINT;
    case itk::ImageIOBase::INT:
      return MI_TYPE_INT;
    case itk::ImageIOBase::FLOAT:
      return MI_TYPE_FLOAT;
    case itk::ImageIOBase::DOUBLE:
      return MI_TYPE_DOUBLE;
    default:
      itkGenericOutputMacro(<< "unhandled ITK data type: " << componentType);
    }
  return MI_TYPE_UNKNOWN;
}

mitype_t ConvertComplexDataTypeToMINC( const itk::ImageIOBase::IOComponentType& componentType )
{
  switch( componentType )
    {
    case itk::ImageIOBase::SHORT:
      return MI_TYPE_SCOMPLEX;
    case itk::ImageIOBase::INT:
      return MI_TYPE_ICOMPLEX;
    case itk::ImageIOBase::FLOAT:
      return MI_TYPE_FCOMPLEX;
    case itk::ImageIOBase::DOUBLE:
      return MI_TYPE_DCOMPLEX;
    default:
      itkGenericOutputMacro(<< "unhandled ITK data type: " << componentType);
    }
  return MI_TYPE_UNKNOWN;
}

/**
 * Copy the start/size information from region to arrays starts,
 * sizes.
 */
void ConvertRegionToMINC( const itk::ImageIORegion& region,
			  unsigned long starts[],
			  unsigned long sizes[] )
{
  for( unsigned int d = 0; d < region.GetImageDimension(); ++d )
    {
    starts[d] = region.GetIndex(d);
    sizes[d] = region.GetSize(d);
    }
}

} // end of unnamed namespace


MINCImageIO::MINCImageIO()
  : m_VolumeValid( false ),
    m_VolumeDimension( 0 )
{
  this->AddSupportedReadExtension( ".mnc" );
  this->AddSupportedReadExtension( ".mnc2" );

  this->AddSupportedWriteExtension( ".mnc" );
  this->AddSupportedWriteExtension( ".mnc2" );
}

MINCImageIO::~MINCImageIO()
{
  this->CloseVolume();
}

void MINCImageIO::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Volume: ";

  if ( m_VolumeValid )
    os << m_Volume;
  else
    os << "(none)";
  os << "\n";
}

bool MINCImageIO::CanReadFile( const char* filename )
{
  mihandle_t volume;

  bool canRead = miopen_volume( filename, MI2_OPEN_READ, &volume ) == MI_NOERROR;

  if ( canRead )
    miclose_volume( volume );

  return canRead;
}

void MINCImageIO::ReadImageInformation()
{
  this->CloseVolume();

  const char* filename = this->GetFileName();
  if ( miopen_volume( filename, MI2_OPEN_READ, &m_Volume ) == MI_ERROR )
    {
    itkExceptionMacro(<< "cannot read file " << filename );
    }

  m_VolumeValid = true;

  this->ReadPixelInformation();
  this->ReadShapeInformation();
  this->ReadImageToWorldInformation();
  this->ComputeStrides();
}

void MINCImageIO::Read( void* buffer )
{
  mitype_t bufferDataType;

  if ( this->GetPixelType() == itk::ImageIOBase::COMPLEX )
    bufferDataType = ConvertComplexDataTypeToMINC( this->GetComponentType() );
  else
    bufferDataType = ConvertScalarDataTypeToMINC( this->GetComponentType() );

  unsigned long* starts = new unsigned long[this->GetNumberOfDimensions()];
  unsigned long* sizes = new unsigned long[this->GetNumberOfDimensions()];
  ConvertRegionToMINC( this->GetIORegion(), starts, sizes );

  if ( miget_real_value_hyperslab( m_Volume, bufferDataType, starts, sizes, buffer ) == MI_ERROR )
    {
    itkExceptionMacro(<< "error reading pixel values");
    }
  
  delete[] starts;
  delete[] sizes;
}

bool MINCImageIO::CanWriteFile( const char* filenameOrig )
{
  std::string filename( filenameOrig );

  // transform filename to lower case to make checks case-insensitive
  std::transform( filename.begin(), filename.end(), filename.begin(), (int(*)(int)) std::tolower );

  std::string::size_type mncPos = filename.rfind(".mnc");
  if ( (mncPos != std::string::npos)
       && (mncPos > 0)
       && (mncPos == filename.length() - 4) )
    {
    return true;
    }

  mncPos = filename.rfind(".mnc2");
  if ( (mncPos != std::string::npos)
       && (mncPos > 0)
       && (mncPos == filename.length() - 5) )
    {
    return true;
    }

  return false;
}

void MINCImageIO::WriteImageInformation()
{
}

void MINCImageIO::Write( const void* buffer )
{
}

void MINCImageIO::ReadPixelInformation()
{
  // Pixel & Component information computed from data type and data
  // class

  miclass_t dataClass;

  if ( miget_data_class( m_Volume, &dataClass ) == MI_ERROR )
    itkExceptionMacro(<< "cannot get data class");

  switch( dataClass )
    {
    case MI_CLASS_REAL:
    case MI_CLASS_INT:
    case MI_CLASS_LABEL:
      this->SetPixelType( itk::ImageIOBase::SCALAR );
      this->SetNumberOfComponents( 1 );
      break;
    case MI_CLASS_COMPLEX:
      this->SetPixelType( itk::ImageIOBase::COMPLEX );
      this->SetNumberOfComponents( 2 );
      break;
    default:
      itkExceptionMacro(<< "unhandled data class: " << dataClass);
    }

  mitype_t dataType = MI_TYPE_UNKNOWN;

  if ( miget_data_type( m_Volume, &dataType ) == MI_ERROR )
    itkExceptionMacro(<< "cannot get data type");

  IOComponentType compType = ConvertDataTypeToITK( dataType );
  if ( compType == UNKNOWNCOMPONENTTYPE )
    itkExceptionMacro(<< "unhandled MINC data type: " << dataType );

  this->SetComponentType( compType );
}

void MINCImageIO::ReadShapeInformation()
{
  int numDimensions;

  if ( miget_volume_dimension_count( m_Volume, 
				     MI_DIMCLASS_ANY, 
				     MI_DIMATTR_REGULARLY_SAMPLED,
				     &numDimensions ) == MI_ERROR )
    {
    itkExceptionMacro(<< "cannot get number of dimensions");
    }

  this->SetNumberOfDimensions( numDimensions );

  delete[] m_VolumeDimension;
  m_VolumeDimension = new midimhandle_t[numDimensions];

  if ( miget_volume_dimensions( m_Volume,
				MI_DIMCLASS_ANY,
				MI_DIMATTR_REGULARLY_SAMPLED,
				MI_DIMORDER_FILE,
				numDimensions,
				m_VolumeDimension ) == MI_ERROR )
    {
    itkExceptionMacro(<< "cannot get dimension meta-data");
    }

  for( int dim = 0; dim < numDimensions; ++dim )
    {
    unsigned int dimSize;
    if ( miget_dimension_size( m_VolumeDimension[dim], &dimSize ) == MI_ERROR )
      {
      itkExceptionMacro(<< "cannot get size of dimension " << dim);
      }
    this->SetDimensions( dim, dimSize );
    }
}

void MINCImageIO::ReadImageToWorldInformation()
{
  for( unsigned int dim = 0; dim < this->GetNumberOfDimensions(); ++dim )
    {
    double spacing;

    if ( miget_dimension_separation( m_VolumeDimension[dim], MI_ORDER_FILE, &spacing ) == MI_ERROR )
      {
      itkExceptionMacro(<< "cannot get spacing of dimension " << dim);
      }

    // MINC allows negative spacing.  We convert to positive spacing
    // and flip the axis direction.
    bool flipAxis = spacing < 0;
    if ( flipAxis ) 
      spacing *= -1;
    this->SetSpacing( dim, spacing );

    double origin;
    if ( miget_dimension_start( m_VolumeDimension[dim], MI_ORDER_FILE, &origin ) == MI_ERROR )
      {
      itkExceptionMacro(<< "cannot get origin of dimension " << dim);
      }

    this->SetOrigin( dim, origin );

    double cosines[3];
    if ( miget_dimension_cosines( m_VolumeDimension[dim], cosines ) == MI_ERROR )
      {
      itkExceptionMacro(<< "cannot get direction cosines of dimension " << dim);
      }

    if ( flipAxis )
      {
      cosines[0] *= -1;
      cosines[1] *= -1;
      cosines[2] *= -1;
      }

    // MINC uses RAS convention for world-space, so X- and
    // Y-coordinates must be flipped to produce the LPS-convention
    // direction
    cosines[0] *= -1;
    cosines[1] *= -1;
    this->SetDirectionFromCosines( dim, cosines );
    }
}

void MINCImageIO::SetDirectionFromCosines( unsigned int dim, double cosines[3] )
{
  std::vector<double> direction;
  direction.resize( this->GetNumberOfDimensions() );

  for( unsigned int i = 0; i < this->GetNumberOfDimensions(); ++i )
    {
    if ( i < 3 )
      direction[i] = cosines[i];
    else
      direction[i] = (dim == i ? 1 : 0);
    }

  this->SetDirection( dim, direction );
}

void MINCImageIO::CloseVolume()
{
  if ( ! m_VolumeValid )
    return;

  m_VolumeValid = false;
  miclose_volume( m_Volume );
  delete[] m_VolumeDimension;
  m_VolumeDimension = 0;
}




} // namespace itk

