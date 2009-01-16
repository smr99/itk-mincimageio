#include <gtest/gtest.h>

#include "itkMINCImageIO.h"
#include "CreateMincFile.h"


typedef itk::MINCImageIO ImageIO;
typedef std::vector<double> DirectionType;


const char* axisOrderArg[] = { "-xyz", "-xzy", "-yxz", "-yzx", "-zxy", "-zyx" };


class MINCImageIOTest : public testing::Test
{
protected:
  ImageIO::Pointer mImageIO;

  // 3D Unit vectors along axes 0, 1, and 2.
  DirectionType mDir0, mDir1, mDir2;

  // 3D Unit vectors in negative directions along axes 0, 1, 2.
  DirectionType mDirNeg0, mDirNeg1, mDirNeg2;

  

  MINCImageIOTest()
    : mImageIO( ImageIO::New() ), 
      mDir0(3), mDir1(3), mDir2(3),
      mDirNeg0(3), mDirNeg1(3), mDirNeg2(3)
  {
    mDir0[0] = 1;
    mDir1[1] = 1;
    mDir2[2] = 1;

    mDirNeg0[0] = -1;
    mDirNeg1[1] = -1;
    mDirNeg2[2] = -1;
  }

  void ReadImageInformation( const char* filename )
  {
    mImageIO->SetFileName( filename );
    mImageIO->ReadImageInformation();
  }

  std::string CreateFile( std::string rawtomincArgs, int dim0, int dim1 )
  {
    rawtomincArgs += " test.mnc";
    std::string fileCreationCommand = createMincFile( rawtomincArgs, dim0, dim1 );
    mImageIO->SetFileName( "test.mnc" );
    mImageIO->ReadImageInformation();
    return fileCreationCommand;
  }

  std::string CreateFile( std::string rawtomincArgs, int dim0, int dim1, int dim2 )
  {
    rawtomincArgs += " test.mnc";
    std::string fileCreationCommand = createMincFile( rawtomincArgs, dim0, dim1, dim2 );
    mImageIO->SetFileName( "test.mnc" );
    mImageIO->ReadImageInformation();
    return fileCreationCommand;
  }

  void SizeTest( std::string fileCreationCommand,
		 itk::ImageIOBase::IOPixelType pixelType,
		 itk::ImageIOBase::IOComponentType compType, 
		 int numPixels,
		 int compPerPixel, 
		 int bytesPerComp )
  {
    int numComponents = numPixels * compPerPixel;
    int numBytes = numComponents * bytesPerComp;

    EXPECT_EQ( pixelType, mImageIO->GetPixelType() ) 
      << fileCreationCommand;
    EXPECT_EQ( compType, mImageIO->GetComponentType() ) 
      << fileCreationCommand;
    EXPECT_EQ( compPerPixel, mImageIO->GetNumberOfComponents() ) 
      << fileCreationCommand;

    EXPECT_EQ( numPixels, mImageIO->GetImageSizeInPixels() ) 
      << fileCreationCommand;
    EXPECT_EQ( numComponents, mImageIO->GetImageSizeInComponents() )   
      << fileCreationCommand;
    EXPECT_EQ( numBytes, mImageIO->GetImageSizeInBytes() ) 
      << fileCreationCommand;
  }

  void SizeTest2D( std::string pixelTypeArg, 
		   itk::ImageIOBase::IOPixelType pixelType,
		   itk::ImageIOBase::IOComponentType compType, 
		   int compPerPixel, 
		   int bytesPerComp )
  {
    int dim0 = 3;
    int dim1 = 4;
    SizeTest( CreateFile( pixelTypeArg, dim0, dim1 ),
	      pixelType,
	      compType,
	      dim0 * dim1,
	      compPerPixel,
	      bytesPerComp );
  }

  void SizeTest3D( std::string pixelTypeArg, 
		   itk::ImageIOBase::IOPixelType pixelType,
		   itk::ImageIOBase::IOComponentType compType, 
		   int compPerPixel, 
		   int bytesPerComp )
  {
    int dim0 = 3;
    int dim1 = 4;
    int dim2 = 7;
    SizeTest( CreateFile( pixelTypeArg, dim0, dim1, dim2 ),
	      pixelType,
	      compType,
	      dim0 * dim1 * dim2,
	      compPerPixel,
	      bytesPerComp );
  }

  template<class Iter>
  void ShapeTest( std::string fileCreationCommand,
		  Iter dimBegin,
		  Iter dimEnd )
  {
    int numDimensions = std::distance( dimBegin, dimEnd );
    
    EXPECT_EQ( numDimensions, mImageIO->GetNumberOfDimensions() ) 
      << fileCreationCommand;

    Iter dim = dimBegin;
    for( int i = 0; dim != dimEnd; ++i, ++dim )
      {
      EXPECT_EQ( *dim, mImageIO->GetDimensions( i ) ) 
	<< "fail at dimension i=" << i << " [" << fileCreationCommand << "]";
      }
  }

  template<class Iter>
  void OriginTest( std::string fileCreationCommand,
		   Iter origin,
		   Iter originEnd )
  {
    for( int i = 0; origin != originEnd; ++i, ++origin )
      {
      EXPECT_DOUBLE_EQ( *origin, mImageIO->GetOrigin( i ) ) 
	<< "fail at dimension i=" << i << " [" << fileCreationCommand << "]";
      }
  }

  template<class Iter>
  void SpacingTest( std::string fileCreationCommand,
		    Iter spacing,
		    Iter spacingEnd )
  {
    for( int i = 0; spacing != spacingEnd; ++i, ++spacing )
      {
      EXPECT_DOUBLE_EQ( *spacing, mImageIO->GetSpacing( i ) ) 
	<< "fail at dimension i=" << i << " [" << fileCreationCommand << "]";
      }
  }

  template<class Iter>
  void DirectionTest( std::string fileCreationCommand,
		      Iter direction,
		      Iter directionEnd )
  {
    int numDimensions = mImageIO->GetNumberOfDimensions();

    for( int i = 0; direction != directionEnd; ++i, ++direction )
      {
      std::vector<double> actual = mImageIO->GetDirection( i );
      EXPECT_EQ( numDimensions, actual.size() )
	<< "fail at axis i=" << i << " [" << fileCreationCommand << "]";
      
      for( int d = 0; d < numDimensions; ++d )
	{
	EXPECT_DOUBLE_EQ( (*direction)[d], actual[d] )
	  << "fail at axis i=" << i << ", d=" << d << " [" << fileCreationCommand << "]";
	}
      }
  }

  // Caller must ensure the region really specifies 6 pixels; e.g. 2x3 region
  template<class TPixel>
  void TestRead6( const itk::ImageIORegion& region,
		  TPixel v0, TPixel v1, TPixel v2, TPixel v3, TPixel v4, TPixel v5 )
  {
    // Before continuing, ensure buffer type is what the ImageIO will produce
    ASSERT_TRUE( typeid(TPixel) == mImageIO->GetComponentTypeInfo() );

    mImageIO->SetIORegion( region );

    TPixel buffer[6];
    mImageIO->Read( &buffer[0] );

    EXPECT_EQ( v0, buffer[0] );
    EXPECT_EQ( v1, buffer[1] );
    EXPECT_EQ( v2, buffer[2] );
    EXPECT_EQ( v3, buffer[3] );
    EXPECT_EQ( v4, buffer[4] );
    EXPECT_EQ( v5, buffer[5] );
  }

  // Caller must ensure the region really specifies 12 pixels; e.g. 2x2x3 region
  template<class TPixel>
  void TestRead12( const itk::ImageIORegion& region,
		   TPixel v0, TPixel v1, TPixel v2, TPixel v3, TPixel v4, TPixel v5,
		   TPixel v6, TPixel v7, TPixel v8, TPixel v9, TPixel v10, TPixel v11 )
  {
    // Before continuing, ensure buffer type is what the ImageIO will produce
    ASSERT_TRUE( typeid(TPixel) == mImageIO->GetComponentTypeInfo() );

    mImageIO->SetIORegion( region );

    TPixel buffer[12];
    mImageIO->Read( &buffer[0] );

    EXPECT_EQ( v0, buffer[0] );
    EXPECT_EQ( v1, buffer[1] );
    EXPECT_EQ( v2, buffer[2] );
    EXPECT_EQ( v3, buffer[3] );
    EXPECT_EQ( v4, buffer[4] );
    EXPECT_EQ( v5, buffer[5] );
    EXPECT_EQ( v6, buffer[6] );
    EXPECT_EQ( v7, buffer[7] );
    EXPECT_EQ( v8, buffer[8] );
    EXPECT_EQ( v9, buffer[9] );
    EXPECT_EQ( v10, buffer[10] );
    EXPECT_EQ( v11, buffer[11] );
  }

};



TEST_F( MINCImageIOTest, CanReadMINCFile )
{
  EXPECT_FALSE( mImageIO->CanReadFile( "" ) );
  EXPECT_FALSE( mImageIO->CanReadFile( "nonexistant.mnc" ) );
  EXPECT_FALSE( mImageIO->CanReadFile( "test1.cpp" ) );

  createMincFile( "test.mnc", 2, 7 );
  EXPECT_TRUE( mImageIO->CanReadFile( "test.mnc" ) );
}

TEST_F( MINCImageIOTest, CanWriteMINCFile )
{
  EXPECT_FALSE( mImageIO->CanWriteFile( "" ) );
  EXPECT_FALSE( mImageIO->CanWriteFile( "blah" ) );
  EXPECT_FALSE( mImageIO->CanWriteFile( ".mnc" ) );
  EXPECT_FALSE( mImageIO->CanWriteFile( ".mnc2" ) );

  EXPECT_TRUE( mImageIO->CanWriteFile( "good.mnc" ) );
  EXPECT_TRUE( mImageIO->CanWriteFile( "good.MNC" ) );
  EXPECT_TRUE( mImageIO->CanWriteFile( "good.mNc" ) );
  EXPECT_TRUE( mImageIO->CanWriteFile( "good.mnc2" ) );
  EXPECT_TRUE( mImageIO->CanWriteFile( "good.MNC2" ) );
  EXPECT_TRUE( mImageIO->CanWriteFile( "good.mNc2" ) );
}

TEST_F( MINCImageIOTest, SizeTest2D )
{
  SizeTest2D( "-osigned -obyte",    itk::ImageIOBase::SCALAR, itk::ImageIOBase::CHAR,   1, 1 );
  SizeTest2D( "-ounsigned -obyte",  itk::ImageIOBase::SCALAR, itk::ImageIOBase::UCHAR,  1, 1 );
  SizeTest2D( "-osigned -oshort",   itk::ImageIOBase::SCALAR, itk::ImageIOBase::SHORT,  1, 2 );
  SizeTest2D( "-ounsigned -oshort", itk::ImageIOBase::SCALAR, itk::ImageIOBase::USHORT, 1, 2 );
  SizeTest2D( "-osigned -oint",     itk::ImageIOBase::SCALAR, itk::ImageIOBase::INT,    1, 4 );
  SizeTest2D( "-ounsigned -oint",   itk::ImageIOBase::SCALAR, itk::ImageIOBase::UINT,   1, 4 );
  SizeTest2D( "-ofloat",            itk::ImageIOBase::SCALAR, itk::ImageIOBase::FLOAT,  1, 4 );
  SizeTest2D( "-odouble",           itk::ImageIOBase::SCALAR, itk::ImageIOBase::DOUBLE, 1, 8 );
}

TEST_F( MINCImageIOTest, SizeTest3D )
{
  SizeTest3D( "-osigned -obyte",    itk::ImageIOBase::SCALAR, itk::ImageIOBase::CHAR,   1, 1 );
  SizeTest3D( "-ounsigned -obyte",  itk::ImageIOBase::SCALAR, itk::ImageIOBase::UCHAR,  1, 1 );
  SizeTest3D( "-osigned -oshort",   itk::ImageIOBase::SCALAR, itk::ImageIOBase::SHORT,  1, 2 );
  SizeTest3D( "-ounsigned -oshort", itk::ImageIOBase::SCALAR, itk::ImageIOBase::USHORT, 1, 2 );
  SizeTest3D( "-osigned -oint",     itk::ImageIOBase::SCALAR, itk::ImageIOBase::INT,    1, 4 );
  SizeTest3D( "-ounsigned -oint",   itk::ImageIOBase::SCALAR, itk::ImageIOBase::UINT,   1, 4 );
  SizeTest3D( "-ofloat",            itk::ImageIOBase::SCALAR, itk::ImageIOBase::FLOAT,  1, 4 );
  SizeTest3D( "-odouble",           itk::ImageIOBase::SCALAR, itk::ImageIOBase::DOUBLE, 1, 8 );
}

TEST_F( MINCImageIOTest, ShapeTest2D )
{
  int shape[] = {3,4};
  
  for( int i = 0; i < 6; ++i )
    {
    SCOPED_TRACE( "ShapeTest2D" );
    ShapeTest( CreateFile( axisOrderArg[i], shape[0], shape[1] ),
	       shape,
	       shape + 2 );
    }
}

TEST_F( MINCImageIOTest, ShapeTest3D )
{
  int shape[] = {3,4,7};
  
  for( int i = 0; i < 6; ++i )
    {
    SCOPED_TRACE( "ShapeTest3D" );
    ShapeTest( CreateFile( axisOrderArg[i], shape[0], shape[1], shape[2] ),
	       shape,
	       shape + 3 );
    }
}

TEST_F( MINCImageIOTest, OriginTest2DUnrotated )
{
  std::string originArgs = "-zxy -xstart -1 -ystart 2 -xstep 3";

  double originXYZ[] = { -1, 2 };
  OriginTest( CreateFile( originArgs, 3, 7 ),
	      originXYZ, originXYZ+2 );
}

TEST_F( MINCImageIOTest, OriginTest2DRotated45 )
{
  // Rotate by 45 degrees w.r.t. world axes
  std::string originArgs = "-zxy -xdircos 0.70711 0.70711 0 -ydircos -0.70711 0.70711 0";

  double origin00[] = { 0, 0 };
  OriginTest( CreateFile( originArgs, 3, 7 ),
	      origin00, origin00+2 );

  double origin10[] = { 0.70711, 0.70711 };
  OriginTest( CreateFile( originArgs + " -xstep 1", 3, 7 ),
	      origin10, origin10+2 );

  double origin01[] = { -0.70711, 0.70711 };
  OriginTest( CreateFile( originArgs + " -ystep 1", 3, 7 ),
	      origin01, origin01+2 );
}

TEST_F( MINCImageIOTest, OriginTest3D )
{
  SCOPED_TRACE( "OriginTest3D" );

  std::string originArgs = "-xstart -1 -ystart -2 -zstart -3 ";

  double originXYZ[] = { -1, -2, -3 };
  OriginTest( CreateFile( originArgs + "-xyz", 3, 7, 5 ),
	      originXYZ, originXYZ+3 );
  
  double originZXY[] = { -3, -1, -2 };
  OriginTest( CreateFile( originArgs + "-zxy", 3, 7, 5 ),
	      originZXY, originZXY+3 );
}

TEST_F( MINCImageIOTest, SpacingTest3D )
{
  SCOPED_TRACE( "SpacingTest3D" );

  std::string spacingArgs = "-xstep 2 -ystep 3 -zstep 1 ";

  double spacingXYZ[] = { 2, 3, 1 };
  SpacingTest( CreateFile( spacingArgs + "-xyz", 3, 7, 2 ),
	       spacingXYZ, spacingXYZ+3 );

  double spacingXZY[] = { 2, 1, 3 };
  SpacingTest( CreateFile( spacingArgs + "-xzy", 3, 7, 2 ),
	       spacingXZY, spacingXZY+3 );
}

TEST_F( MINCImageIOTest, DirectionTest3D )
{
  SCOPED_TRACE( "DirectionTest3D" );

  // MINC uses RAS convention, so the MINC direction cosines 
  // must be rotated 90 deg. about Z to obtain ITK's LPS equivalents

  std::vector<double> directionXYZ[] = { mDirNeg0, mDirNeg1, mDir2 };
  DirectionTest( CreateFile( "-xyz", 3, 9, 2 ),
		 directionXYZ, directionXYZ+3 );

  std::vector<double> directionYXZ[] = { mDirNeg1, mDirNeg0, mDir2 };
  DirectionTest( CreateFile( "-yxz", 3, 9, 2 ),
		 directionYXZ, directionYXZ+3 );
}

TEST_F( MINCImageIOTest, ReadTestFullImage2D )
{
  SCOPED_TRACE( "ReadTestFullImage2D" );

  // Create file filled with consecutive numbers starting with
  // 0.  The 2D file has axes X and Y with Y-coords varying fastest.
  // Thus the image array (with X pointing right and Y down) is:
  //
  //        0  3    
  //        1  4
  //        2  5
  //
  CreateFile( "-zxy -ounsigned -obyte -real_range 0 255", 2, 3 );

  // Create a region to cover the entire file
  itk::ImageIORegion region( 2 );
  region.SetSize( 0, 2 );
  region.SetSize( 1, 3 );

  TestRead6<unsigned char>( region, 0, 1, 2, 3, 4, 5 );
}

TEST_F( MINCImageIOTest, ReadTestSubImage2D )
{
  SCOPED_TRACE( "ReadTestSubImage2D" );

  // File is created and filled with consecutive numbers starting with
  // 0.  The 2D file has axes X and Y with Y-coords varying fastest.
  // Thus the image array (with X pointing right and Y down) is:
  //
  //        0   4   8  12
  //        1   5   9  13
  //        2   6  10  14
  //        3   7  11  15
  //
  CreateFile( "-zxy -ounsigned -obyte -real_range 0 255", 4, 4 );

  // Create a 2x3 region
  itk::ImageIORegion region( 2 );
  region.SetSize( 0, 2 );
  region.SetSize( 1, 3 );

  // Read starting at index (0,0)
  TestRead6<unsigned char>( region, 0, 1, 2, 4, 5, 6 );

  // Read starting at index (1,1)
  region.SetIndex( 0, 1 );
  region.SetIndex( 1, 1 );
  TestRead6<unsigned char>( region, 5, 6, 7, 9, 10, 11 );

  // Read from 3x2 region, starting at index (1,2)
  region.SetSize( 0, 3 );
  region.SetSize( 1, 2 );
  region.SetIndex( 0, 1 );
  region.SetIndex( 1, 2 );
  TestRead6<unsigned char>( region, 6, 7, 10, 11, 14, 15 );
  
}

TEST_F( MINCImageIOTest, ReadTestFullImage3D )
{
  SCOPED_TRACE( "ReadTestFullImage3D" );

  // Create file filled with consecutive numbers starting with
  // 0.  Image X-axis varies slowest, Z the fastest.
  // Thus the image array (with Y pointing right and Z down) is:
  //
  // X=0
  //        0  2  4
  //        1  3  5
  //
  // X=1
  //        6  8  10
  //        7  9  11
  //
  CreateFile( "-xyz -ounsigned -obyte -real_range 0 255", 2, 3, 2 );

  // Create a region to cover the entire file
  itk::ImageIORegion region( 3 );
  region.SetSize( 0, 2 );
  region.SetSize( 1, 3 );
  region.SetSize( 2, 2 );

  TestRead12<unsigned char>( region, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 );
}

TEST_F( MINCImageIOTest, ReadTestSubImage3D )
{
  SCOPED_TRACE( "ReadTestSubImage3" );

  // Create file filled with consecutive numbers starting with
  // 0.  Image X-axis varies slowest, Z the fastest.
  // Thus the image array (with Y pointing right and Z down) is:
  //
  // X=0
  //        0  2  4
  //        1  3  5
  //
  // X=1
  //        6  8  10
  //        7  9  11
  //
  CreateFile( "-xyz -ounsigned -obyte -real_range 0 255", 2, 3, 2 );

  // Create a 1x3x2 region
  itk::ImageIORegion region( 3 );
  region.SetSize( 0, 1 );
  region.SetSize( 1, 3 );
  region.SetSize( 2, 2 );

  // Read starting at index (0,0)
  TestRead6<unsigned char>( region, 0, 1, 2, 3, 4, 5 );

  // Read starting at index (1,0,0)
  region.SetIndex( 0, 1 );
  region.SetIndex( 1, 0 );
  region.SetIndex( 2, 0 );
  TestRead6<unsigned char>( region, 6, 7, 8, 9, 10, 11 );

  // Read from 2x3x1 region, starting at index (0,0)
  region.SetSize( 0, 2 );
  region.SetSize( 1, 3 );
  region.SetSize( 2, 1 );
  region.SetIndex( 0, 0 );
  region.SetIndex( 1, 0 );
  region.SetIndex( 2, 0 );
  TestRead6<unsigned char>( region, 0, 2, 4, 6, 8, 10 );
  
}

TEST_F( MINCImageIOTest, ReadTestRescale )
{
  SCOPED_TRACE( "ReadTestRescale" );

  CreateFile( "-xyz -ounsigned -obyte -real_range 0 1024", 8, 8, 16 );

  // Create a region to cover the entire file
  itk::ImageIORegion region( 3 );
  region.SetSize( 0, 1 );
  region.SetSize( 1, 1 );
  region.SetSize( 2, 6 );

  TestRead6<unsigned short>( region, 0, 4, 8, 12, 16, 20 );
}
