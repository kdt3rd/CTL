
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <stdint.h>

struct format_t
{
	format_t( void )
			: driver( NULL ), ext( NULL ),
			  bps( 0 ), src_bps( 0 ), squish( false ), descriptor( 0 )
	{}
	format_t(const char *_ext, uint8_t _bps)
			: driver( NULL ), ext( _ext ), dps( _bps ),
			  src_bps( 0 ), squish( false ), descriptor( 0 )
	{}

	const char *driver;
	const char *ext;
	uint8_t bps;
	uint8_t src_bps;
	bool squish; // remove alpha channel.
	uint8_t descriptor; // DPX enumeration, plus the following values:
	                    // 157 - XYZ
	                    // 158 - XYZA
	                    // 159 - YA
	                    // 160 - RA
	                    // 161 - GA
	                    // 162 - BA
};

#include <dpx.hh>
#ifdef HAVE_OPENEXR
# include <ImfInputFile.h>
# include <ImfOutputFile.h>
# include <ImfRgbaFile.h>
# include <ImfArray.h>
# include <ImfHeader.h>
# include <ImfChannelList.h>
# include <Iex.h>
#endif // HAVE_OPENEXR
#ifdef HAVE_ACESFILE
# include <aces_Writer.h>
# include <stdexcept>
# include <half.h>
#endif
#ifdef HAVE_LIBTIFF
# include <tiff.h>
# include <tiffio.h>
# include <sys/param.h>
# include <math.h>
# include <alloca.h>
#endif

static bool
dpx_read( const std::string &name, float scale,
		  ctl::dpx::fb<float> &pixels, format_t &format )
{
	std::ifstream file( name.c_str() );
	ctl::dpx dpxheader;

	if( ! ctl::dpx::check_magic( &file ) )
		return false;
	
	dpxheader.read( &file );
	dpxheader.read( &file, 0, &pixels, scale );

	format.src_bps = dpxheader.elements[0].bits_per_sample;
	pixels->swizzle( dpxheader.elements[0].descriptor, FALSE );

	return true;
}

static void
dpx_write( const std::string &name, float scale,
		   const ctl::dpx::fb<float> &pixels,
		   const format_t &format )
{
	std::ofstream file( name.c_str() );
	ctl::dpx dpxheader;

	dpxheader.elements[0].data_sign = 0;
	dpxheader.elements[0].bits_per_sample = format->bps;
	dpxheader.write( &file, 0, pixels, scale );
	dpxheader.write( &file );	
}

#ifdef HAVE_OPENEXR

static bool
exr_read( const std::string &name, float scale,
		  ctl::dpx::fb<float> &pixels, format_t &format)
{
	{
		std::ifstream ins( name.c_str() );
		unsigned int magic, endian;

		ins.open( name );

		ins.read( (char *)&magic, sizeof(magic) );
		if ( magic != 0x01312f76 )
			return false;
	}
	
	Imf::InputFile file( name.c_str() );
	Imath::Box2i dw = file.header().dataWindow();

	if ( file.header().channels().begin().channel().type == Imf::HALF )
		format->src_bps = 16;
	else
		format->src_bps = 32;
		
	int width = dw.max.x - dw.min.x + 1;
	int height = dw.max.y - dw.min.y + 1;
	
	pixels.init( width, height, 4 );
	Imf::PixelType pixelType = Imf::FLOAT;
	
	int xstride = sizeof( *pixels.ptr() ) * pixels->depth();
	int ystride = sizeof( *pixels.ptr() ) * pixels->depth() * pixels->width();
	
	Imf::FrameBuffer frameBuffer;
	frameBuffer.insert( "R", Imf::Slice( pixelType,
										 (char *) pixels.ptr(),
										 xstride, ystride,
										 1, 1, 0.0));
	
	frameBuffer.insert( "G", Imf::Slice( pixelType,
										 (char *)( pixels.ptr() + 1 ),
										 xstride, ystride,
										 1, 1, 0.0 ) );
	
	frameBuffer.insert( "B", Imf::Slice( pixelType,
										 (char *)( pixels->ptr() + 2 ),
										 xstride, ystride,
										 1, 1, 0.0 ) );
	
	frameBuffer.insert( "A", Imf::Slice( pixelType,
										 (char *)( pixels->ptr() + 3 ),
										 xstride, ystride,
										 1, 1, 1.0 ) );
	
	file.setFrameBuffer( frameBuffer );
	file.readPixels( dw.min.y, dw.max.y );
	
	if ( scale == 0.0 || scale == 1.0 )
		return true;

	float *p=pixels->ptr();
	for(uint64_t i=0; i<pixels->count(); i++)
	{
		*p=*p*scale;
		p++;
	}
	return true;
}

static void
exr_write32( const std::string &name, float scale,
			 const ctl::dpx::fb<float> &pixels,
			 const Compression &compression )
{
	int depth = pixels.depth();
	float width = pixels.width();
	float height = pixels.height();
	const float *pixelPtr = pixels.ptr();

	ctl::dpx::fb<float> scaled_pixels;
	if ( scale != 0.0 && scale != 1.0 )
	{
		scaled_pixels.init(pixels.height(), pixels.width(), pixels.depth());
		scaled_pixels.alpha(1.0);
		
		const float *fIn=pixels.ptr();
		float *out=scaled_pixels.ptr();
		
		for(uint64_t i=0; i<pixels.count(); i++)
			*(out++)=*(fIn++)/scale;
		
		depth = scaled_pixels.depth();
		width = scaled_pixels.width();
		height = scaled_pixels.height();
		pixelPtr = scaled_pixels.ptr();
	}
	
	Imf::PixelType pixelType = Imf::FLOAT;
	
	Imf::Header header( width, height );
	header.compression() = (Imf::Compression)compression.exrCompressionScheme;
	
	header.channels().insert( "R", Imf::Channel(pixelType) );
	header.channels().insert( "G", Imf::Channel(pixelType) );
	header.channels().insert( "B", Imf::Channel(pixelType) );
	
	if ( depth == 4 )
		header.channels().insert( "A", Imf::Channel(pixelType) );
	
	Imf::OutputFile file( name.c_str(), header );
	
	Imf::FrameBuffer frameBuffer;
	
	int xstride = sizeof (*pixelPtr) * depth;
	int ystride = sizeof (*pixelPtr) * depth * width;
	
	frameBuffer.insert( "R", Imf::Slice( pixelType, (char *)pixelPtr,
										 xstride, ystride ) );
	frameBuffer.insert( "G", Imf::Slice( pixelType, (char *)( pixelPtr + 1 ),
										 xstride, ystride ) );
	frameBuffer.insert( "B", Imf::Slice( pixelType, (char *)( pixelPtr + 2 ),
										 xstride, ystride ) );
	if (depth == 4)
		frameBuffer.insert( "A", Imf::Slice( pixelType, (char *)( pixelPtr + 3 ),
											 xstride, ystride ) );
	
	file.setFrameBuffer( frameBuffer );
	file.writePixels( height );
}

static void
exr_write16( const std::string &name, float scale,
			 const ctl::dpx::fb<float> &pixels,
			 const Compression &compression )
{
	if (scale == 0.0) scale = 1.0;

	ctl::dpx::fb<half> scaled_pixels;
	scaled_pixels.init( pixels.height(), pixels.width(), pixels.depth() );
	scaled_pixels.alpha( 1.0 );

	const float *fIn=pixels.ptr();
	half *out=scaled_pixels.ptr();
	
	for ( size_t i = 0, N = pixels.count(); i != N; ++i )
		*(out++) = *(fIn++)/scale;
	
	uint8_t channels = scaled_pixels.depth();
	const half *in = scaled_pixels.ptr();
	
	Imf::RgbaOutputFile file( name.c_str(), pixels.width(), pixels.height(),
							  channels == 4 ? Imf::WRITE_RGBA : Imf::WRITE_RGB,
							  1, Imath::V2f (0, 0), 1,
							  Imf::INCREASING_Y,
							  (Imf::Compression)compression.exrCompressionScheme );

	file.setFrameBuffer( (Imf::Rgba *)in, 1, pixels.width() );
	file.writePixels( pixels.height() );
}

static void
exr_write( const std::string &name, float scale,
		   const ctl::dpx::fb<float> &pixels,
		   const format_t &format,
		   const Compression &compression)
{
	switch ( format.bps )
	{
		case 16:
			exr_write16( name, scale, pixels, compression );
			break;
		case 32:
			exr_write32( name, scale, pixels, compression );
			break;
		default:
			THROW(Iex::ArgExc, "EXR files only support 16 or 32 bps at the moment.");
	}
}

#endif // HAVE_OPENEXR

#ifdef HAVE_ACESFILE

static void
aces_write( const std::string &name, float scale, 
			uint32_t width, uint32_t height, uint32_t channels,
			const float *pixels,
			const format_t &format )
{
	std::vector<half_bytes> scaled_pixels;
	
	if (scale == 0.0f) scale = 1.0f;
	const float *in = pixels;
		
	scaled_pixels.resize( height * width * channels );
	half_bytes *out = &scaled_pixels[0];
	for ( size_t i = 0, N = scaled_pixedls.size(); i != N; ++i )
	{
		half tmpV( *(in++) / scale );
		*(out++)=tmpV.bits();
	}

	half_bytes *in = &scaled_pixels[0];
	
	std::vector<std::string> filenames;
	filenames.push_back( name );
	
	aces_Writer x;
	
	MetaWriteClip writeParams;
	
	writeParams.duration				= 1;	
	writeParams.outputFilenames			= filenames;
	
	writeParams.outputRows				= height;
	writeParams.outputCols				= width;	
	
	writeParams.hi = x.getDefaultHeaderInfo();	
	writeParams.hi.originalImageFlag	= 1;	
	writeParams.hi.software				= "ctlrender";

	writeParams.hi.channels.clear();
	switch ( channels )
	{
		case 3:
			writeParams.hi.channels.resize(3);
			writeParams.hi.channels[0].name = "B";
			writeParams.hi.channels[1].name = "G";
			writeParams.hi.channels[2].name = "R";
			break;
		case 4:
			writeParams.hi.channels.resize(4);
			writeParams.hi.channels[0].name = "A";
			writeParams.hi.channels[1].name = "B";
			writeParams.hi.channels[2].name = "G";
			writeParams.hi.channels[3].name = "R";
			break;
		case 6:
			throw std::invalid_argument("Stereo RGB support not yet implemented");
//			writeParams.hi.channels.resize(6);
//			writeParams.hi.channels[0].name = "B";
//			writeParams.hi.channels[1].name = "G";
//			writeParams.hi.channels[2].name = "R";
//			writeParams.hi.channels[3].name = "left.B";
//			writeParams.hi.channels[4].name = "left.G";
//			writeParams.hi.channels[5].name = "left.R";
//			break;
		case 8:
			throw std::invalid_argument("Stereo RGB support not yet implemented");
//			writeParams.hi.channels.resize(8);
//			writeParams.hi.channels[0].name = "A";
//			writeParams.hi.channels[1].name = "B";
//			writeParams.hi.channels[2].name = "G";
//			writeParams.hi.channels[3].name = "R";
//			writeParams.hi.channels[4].name = "left.A";
//			writeParams.hi.channels[5].name = "left.B";
//			writeParams.hi.channels[6].name = "left.G";
//			writeParams.hi.channels[7].name = "left.R";
//			break;
		default:
			throw std::invalid_argument("Only RGB, RGBA or stereo RGB[A] file supported");
			break;
	}

	DynamicMetadata dynamicMeta;
	dynamicMeta.imageIndex = 0;
	dynamicMeta.imageCounter = 0;
	
	x.configure( writeParams );
	x.newImageObject( dynamicMeta );		

	for ( uint32 row = 0; row < height; row++)
	{
		half_bytes *rgbData = in + width*channels*row;
		x.storeHalfRow( rgbData, row ); 
	}

	x.saveImageObject();	
}

#endif // HAVE_ACESFILE

#ifdef HAVE_LIBTIFF
static void
tiff_interleave_int8( float *o, int offset, float scale,
						  T *r, int r_stride, T *g, int g_stride,
						  T *b, int b_stride, T *a, int a_stride,
						  uint32_t width )
{
	if ( scale == 0.0 )
		scale = 255.F;

	for ( uint32_t i = 0; i != width; ++i )
	{
		if ( r )
		{
			f=*r+offset;
			r=r+r_stride;
			*(o++)=f/scale;
		}
		if( g )
		{
			f=*g+offset;
			g=g+g_stride;
			*(o++)=f/scale;
		}
		if( b )
		{
			f=*b+offset;
			b=b+b_stride;
			*(o++)=f/scale;
		}
		if( a )
		{
			f=*a+offset;
			a=a+a_stride;
			*(o++)=f/scale;
		}
	}
}
						   
static void
tiff_interleave_int16( float *o, uint16_t offset, float scale,
					   uint16_t *r, int r_stride, uint16_t *g, int g_stride,
					   uint16_t *b, int b_stride, uint16_t *a, int a_stride,
					   uint32_t width )
{
	if ( scale == 0.0 )
		scale = 65535.F;

	for ( uint32_t i = 0; i != width; ++i )
	{
		if ( r )
		{
			f=*r+offset;
			r=r+r_stride;
			*(o++)=f/scale;
		}
		if ( g )
		{
			f=*g+offset;
			g=g+g_stride;
			*(o++)=f/scale;
		}
		if ( b )
		{
			f=*b+offset;
			b=b+b_stride;
			*(o++)=f/scale;
		}
		if ( a )
		{
			f=*a+offset;
			a=a+a_stride;
			*(o++)=f/scale;
		}
	}
}

static void
tiff_interleave_float( float *o, float scale,
					   float *r, int r_stride, float *g, int g_stride,
					   float *b, int b_stride, float *a, int a_stride,
					   uint32_t width )
{
	if ( scale == 0.0 )
		scale = 1.0;

	for ( uint32_t i = 0; i != width; ++i )
	{
		if ( r )
		{
			f=*r;
			r=r+r_stride;
			*(o++)=f/scale;
		}
		if ( g )
		{
			f=*g;
			g=g+g_stride;
			*(o++)=f/scale;
		}
		if ( b )
		{
			f=*b;
			b=b+b_stride;
			*(o++)=f/scale;
		}
		if ( a )
		{
			f=*a;
			a=a+a_stride;
			*(o++)=f/scale;
		}
	}
}

static void
ErrorHandler( const char *module, const char *fmt, va_list ap )
{
	fprintf( stderr, "Unable to read tiff file: " );
	vfprintf( stderr, fmt, ap );
}

static void
WarningHandler(const char *module, const char *fmt, va_list ap)
{
//	fprintf(stderr, "tiff wrn: %s - ");
//	vfprintf(stderr, fmt, ap);
}

static void
tiff_read_multiplane( TIFF *t, float scale, ctl::dpx::fb<float> &pixels )
{
	uint8_t *scanline_buffer_uint8[4];
	uint16_t *scanline_buffer_uint16[4];
	float *scanline_buffer_float[4];
	uint16_t samples_per_pixel;
	uint16_t bits_per_sample;
	uint32_t w;
	uint32_t h;
	uint16_t sample_format;
	uint16_t offset;
	uint16_t orientation;
	tsize_t scanline_size;
	float *row_ptr;
	uint32_t row;
	uint32_t orientation_offset;
	uint16_t d;

	TIFFGetFieldDefaulted( t, TIFFTAG_IMAGEWIDTH, &w );
	TIFFGetFieldDefaulted( t, TIFFTAG_IMAGELENGTH, &h );
	TIFFGetFieldDefaulted( t, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel );
	TIFFGetFieldDefaulted( t, TIFFTAG_BITSPERSAMPLE, &bits_per_sample );
	TIFFGetFieldDefaulted( t, TIFFTAG_SAMPLEFORMAT, &sample_format );
	TIFFGetFieldDefaulted( t, TIFFTAG_ORIENTATION, &orientation );

	pixels.init( w, h, samples_per_pixel );

	orientation_offset = 0;
	if ( orientation == ORIENTATION_LEFTTOP )
	{
		// We only deal with the bottom->top flip, not the other orientation
		// modes (the actual check for this is in tiff_read).
		orientation_offset = (uint32_t)-h;
	}

	scanline_size = TIFFScanlineSize(t);
	if ( bits_per_sample == 8 )
	{
		for(row=0; row<4; row++) {
			if(row<samples_per_pixel) {
				scanline_buffer_uint8[row]=(uint8_t *)alloca(scanline_size);
				for(d=0; d<w; d++) {
					scanline_buffer_uint8[row][d]= row==3 ? 255 : 0;
				}
			} else {
				scanline_buffer_uint8[row]=NULL;
			}
		}
		for(;row<4; row++) {
		}
		offset=0;
		if(sample_format==2) {
			offset=1<<7;
		}
		for(row=0; row<h; row++) {
			for(d=0; d<samples_per_pixel; d++) {
				TIFFReadScanline(t, scanline_buffer_uint8[d],
								 row+orientation_offset, d);
			}
			row_ptr=pixels->ptr()+row*pixels->width()*pixels->depth();
			tiff_interleave_int8(row_ptr, offset, scale,
								 scanline_buffer_uint8[0], 1,
								 scanline_buffer_uint8[1], 1,
								 scanline_buffer_uint8[2], 1,
								 scanline_buffer_uint8[3], 1,
								 w);
		}
	} else if(bits_per_sample==16) {
		for(row=0; row<4; row++) {
			if(row<samples_per_pixel) {
				scanline_buffer_uint16[row]=(uint16_t *)alloca(scanline_size);
				for(d=0; d<w; d++) {
					scanline_buffer_uint16[row][d]= row==3 ? 65535 : 0;
				}
			} else {
				scanline_buffer_uint16[row]=NULL;
			}
		}
		offset=0;
		if(sample_format==2) {
			offset=1<<15;
		}
		for(row=0; row<h; row++) {
			for(d=0; d<samples_per_pixel; d++) {
				TIFFReadScanline(t, scanline_buffer_uint16[d],
								 row+orientation_offset, d);
			}
			row_ptr=pixels->ptr()+row*pixels->width()*pixels->depth();
			tiff_interleave_int16(row_ptr, offset, scale,
								  scanline_buffer_uint16[0], 1,
								  scanline_buffer_uint16[1], 1,
								  scanline_buffer_uint16[2], 1,
								  scanline_buffer_uint16[3], 1,
								  w);
		}
	} else if(sample_format==3) {
		for(row=0; row<4; row++) {
			if(row<samples_per_pixel) {
				scanline_buffer_float[row]=(float *)alloca(scanline_size);
				for(d=0; d<w; d++) {
					scanline_buffer_float[row][d]= row==3 ? 1.0 : 0.0;
				}
			} else {
				scanline_buffer_float[row]=NULL;
			}
		}
		for(row=0; row<h; row++) {
			for(d=0; d<samples_per_pixel; d++) {
				TIFFReadScanline(t, scanline_buffer_float[d],
								 row+orientation_offset, d);
			}
			row_ptr=pixels->ptr()+row*pixels->width()*pixels->depth();
			tiff_interleave_float(row_ptr, scale,
								  scanline_buffer_float[0], 1,
								  scanline_buffer_float[1], 1,
								  scanline_buffer_float[2], 1,
								  scanline_buffer_float[3], 1,
								  w);
		}
	}
}

static void
tiff_read_interleaved( TIFF *t, float scale, ctl::dpx::fb<float> &pixels )
{
	uint8_t *scanline_buffer_uint8;
	uint16_t *scanline_buffer_uint16;
	float *scanline_buffer_float;
	uint16_t samples_per_pixel;
	uint16_t bits_per_sample;
	uint32_t w;
	uint32_t h;
	uint16_t sample_format;
	uint16_t offset;
	uint32_t row;
	float *row_ptr;

	TIFFGetFieldDefaulted(t, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetFieldDefaulted(t, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetFieldDefaulted(t, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
	TIFFGetFieldDefaulted(t, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
	TIFFGetFieldDefaulted(t, TIFFTAG_SAMPLEFORMAT, &sample_format);
	pixels.init(w, h, samples_per_pixel);

	if(bits_per_sample==8) {
		scanline_buffer_uint8=(uint8_t *)alloca(TIFFScanlineSize(t));
		offset=0;
		if(sample_format==2) {
			offset=127;
		}
		for(row=0; row<h; row++) {
			TIFFReadScanline(t, scanline_buffer_uint8, row, 0);
			row_ptr=pixels.ptr()+row*pixels.width()*pixels.depth();
			if(samples_per_pixel==3) {
				tiff_interleave_int8(row_ptr, offset, scale,
									 scanline_buffer_uint8+0, 3,
									 scanline_buffer_uint8+1, 3,
									 scanline_buffer_uint8+2, 3,
									 NULL, 0,
									 w);
			} else {
				tiff_interleave_int8(row_ptr, offset, scale,
									 scanline_buffer_uint8+0, 4,
									 scanline_buffer_uint8+1, 4,
									 scanline_buffer_uint8+2, 4,
									 scanline_buffer_uint8+3, 4,
									 w);
			}
		}
	} else if(bits_per_sample==16) {
		scanline_buffer_uint16=(uint16_t *)alloca(TIFFScanlineSize(t));
		offset=0;
		if(sample_format==2) {
			offset=32767;
		}
		for(row=0; row<h; row++) {
			TIFFReadScanline(t, scanline_buffer_uint16, row, 0);
			row_ptr=pixels.ptr()+row*pixels.width()*pixels.depth();
			if(samples_per_pixel==3) {
				tiff_interleave_int16(row_ptr, offset, scale,
									  scanline_buffer_uint16+0, 3,
									  scanline_buffer_uint16+1, 3,
									  scanline_buffer_uint16+2, 3,
									  NULL, 0,
									  w);
			} else {
				tiff_interleave_int16(row_ptr, offset, scale,
									  scanline_buffer_uint16+0, 4,
									  scanline_buffer_uint16+1, 4,
									  scanline_buffer_uint16+2, 4,
									  scanline_buffer_uint16+3, 4,
									  w);
			}
		}
	} else if(sample_format==3) {
		scanline_buffer_float=(float *)alloca(TIFFScanlineSize(t));
		for(row=0; row<h; row++) {
			TIFFReadScanline(t, scanline_buffer_float, row, 0);
			row_ptr=pixels.ptr()+row*pixels.width()*pixels.depth();
			if(samples_per_pixel==3) {
				tiff_interleave_float(row_ptr, scale,
									  scanline_buffer_float+0, 3,
									  scanline_buffer_float+1, 3,
									  scanline_buffer_float+2, 3,
									  NULL, 0,
									  w);
			} else {
				tiff_interleave_float(row_ptr, scale,
									  scanline_buffer_float+0, 4,
									  scanline_buffer_float+1, 4,
									  scanline_buffer_float+2, 4,
									  scanline_buffer_float+3, 4,
									  w);
			}
		}
	}
}


////////////////////////////////////////


static void
tiff_read_failsafe( TIFF *t, float scale, ctl::dpx::fb<float> &pixels )
{
	uint8_t *temp_buffer;
	uint8_t *flip;
	uint32_t i;
	uint32_t w, h;

	TIFFGetFieldDefaulted( t, TIFFTAG_IMAGEWIDTH, &w );
	TIFFGetFieldDefaulted( t, TIFFTAG_IMAGELENGTH, &h );
	pixels.init( w, h, 4 );

	temp_buffer = (uint8_t *)alloca( w * h * 4 );
	TIFFReadRGBAImage( t, w, h, (uint32 *)temp_buffer, 0 );

	for ( uint32_t i = 0; i != h; ++i )
	{
		flip = temp_buffer + sizeof(uint32_t) * w * ( h - i - 1 );
		tiff_interleave_int8( pixels.ptr() + w * i * 4, 0, scale,
							  flip+0, 4, flip+1, 4, flip+2, 4, flip+3, 4, w );
	}
}

static bool
tiff_read( const std::string &name, float scale,
		   ctl::dpx::fb<float> &pixels,
		   format_t &format )
{
	TIFF *t;
	uint16_t samples_per_pixel;
	uint16_t bits_per_sample;
	uint16_t sample_format;
	uint16_t planar_config;
	uint16_t photometric;
	uint16_t orientation;

	TIFFSetErrorHandler( ErrorHandler );
	TIFFSetWarningHandler( WarningHandler );

	t = TIFFOpen( name.c_str(), "r" );
	if ( ! t )
		return false;

	TIFFGetFieldDefaulted( t, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel );
	TIFFGetFieldDefaulted( t, TIFFTAG_BITSPERSAMPLE, &bits_per_sample );
	format->src_bps = bits_per_sample;
	TIFFGetFieldDefaulted( t, TIFFTAG_SAMPLEFORMAT, &sample_format );
	TIFFGetFieldDefaulted( t, TIFFTAG_PHOTOMETRIC, &photometric );
	TIFFGetFieldDefaulted( t, TIFFTAG_ORIENTATION, &orientation );

	//	tiff_read_failsafe(t, scale, pixels);
	//	return TRUE;

	if ( !(bits_per_sample==16 && sample_format<3) &&
		 !(bits_per_sample==32 && sample_format==3) &&
		 photometric!=PHOTOMETRIC_RGB &&
		 orientation!=ORIENTATION_TOPLEFT &&
		 orientation!=ORIENTATION_BOTLEFT )
	{
		if ( bits_per_sample != 8 )
		{
			fprintf( stderr,
						 "falling back to failsafe TIFF reader. Reading "
						 "as \n8 bits per sample RGBA.\n" );
		}
		tiff_read_failsafe(t, scale, pixels);
		TIFFClose(t);
		return true;
	}

	TIFFGetField(t, TIFFTAG_PLANARCONFIG, &planar_config);
	if(planar_config==PLANARCONFIG_CONTIG) {
		tiff_read_interleaved(t, scale, pixels);
	} else if(planar_config==PLANARCONFIG_SEPARATE) {
		tiff_read_multiplane(t, scale, pixels);
	}

	TIFFClose(t);

	return TRUE;
}

static void
tiff_write( const std::string &name, float scale,
			const ctl::dpx::fb<float> &pixels,
			const format_t &format )
{
	TIFF *t;
	uint16_t bits_per_sample;
	tdata_t scanline_buffer;
	uint32_t y;
	uint8_t channel;
	const float *row;

	TIFFSetErrorHandler(ErrorHandler);
	TIFFSetWarningHandler(WarningHandler);

	bits_per_sample=format->bps;
	if(format->bps<=8) {
		bits_per_sample=8;
	} else if(format->bps<=16) {
		bits_per_sample=16;
	} else if(format->bps!=32) {
		THROW(Iex::ArgExc, "TIFF files can only support files with <=16 bps "
			  "(integer) or 32 bps (float).");
	}

	t=TIFFOpen(name, "w");
	if(t==NULL) {
		// What went wrong
	}

	TIFFSetField(t, TIFFTAG_SAMPLESPERPIXEL, pixels.depth());
	TIFFSetField(t, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
	TIFFSetField(t, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(t, TIFFTAG_IMAGEWIDTH, pixels.width());
	TIFFSetField(t, TIFFTAG_IMAGELENGTH, pixels.height());
	TIFFSetField(t, TIFFTAG_ROWSPERSTRIP, 1);
	TIFFSetField(t, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	// Worst case...
	scanline_buffer=alloca(sizeof(float)*pixels.depth()*pixels.width());

	if ( bits_per_sample == 8 )
	{
		TIFFSetField(t, TIFFTAG_SAMPLEFORMAT, 1);
		for(y=0; y<pixels.height(); y++) {
			row=pixels.ptr()+y*pixels.width()*pixels.depth();
			ctl::dpx::convert( reinterpret_cast<uint8_t *>( scanline_buffer ), row,
							   0.0, //scale
							   pixels.depth() * pixels.width() );
			TIFFWriteScanline(t, scanline_buffer, y, channel);
		}
	}
	else if ( bits_per_sample == 16 )
	{
		TIFFSetField(t, TIFFTAG_SAMPLEFORMAT, 1);
		for(y=0; y<pixels.height(); y++) {
			row=pixels.ptr()+y*pixels.width()*pixels.depth();
			ctl::dpx::convert( reinterpret_cast<uint16_t *>( scanline_buffer ), row,
							   0.0, //scale
							   pixels.depth() * pixels.width() );
			TIFFWriteScanline(t, scanline_buffer, y, channel);
		}
	}
	else if(bits_per_sample==32)
	{
		TIFFSetField(t, TIFFTAG_SAMPLEFORMAT, 3);
		for(y=0; y<pixels.height(); y++) {
			row=pixels.ptr()+y*pixels.width()*pixels.depth();
			ctl::dpx::convert( reinterpret_cast<float *>( scanline_buffer ), row,
							   1.0, //scale
							   pixels.depth() * pixels.width() );
			TIFFWriteScanline(t, scanline_buffer, y, channel);
		}
	}

	TIFFClose(t);
}

#endif // HAVE_LIBTIFF


class proc_thread_pool
{
public:
	proc_thread_pool( size_t nThreads = 0 );

	void run_rrt( ctl::dpx::fb<float> &image_buffer,
		float rInDefault = 0.F,
		float gInDefault = 0.F,
		float bInDefault = 0.F,
		float aInDefault = 1.F );
private:
	static void thread_loop( void *me );

	pthread_cond_t myCondition;
	pthread_mutex_t myLock;
};
