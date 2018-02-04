#ifndef JPEG_DECODER_H
#define JPEG_DECODER_H

namespace jpgd
{
	static int pippo;
	// Loads a JPEG image from a memory buffer or a file.
	// req_comps can be 1 (grayscale), 3 (RGB), or 4 (RGBA).
	// On return, width/height will be set to the image's dimensions, and actual_comps will be set to the either 1 (grayscale) or 3 (RGB).
	// Notes: For more control over where and how the source data is read, see the decompress_jpeg_image_from_stream() function below, or call the jpeg_decoder class directly.
	// Requesting a 8 or 32bpp image is currently a little faster than 24bpp because the jpeg_decoder class itself currently always unpacks to either 8 or 32bpp.
	unsigned char *decompress_jpeg_image_from_memory(const unsigned char *pSrc_data, int src_data_size, int *width, int *height, int *actual_comps, int req_comps);
	unsigned char *decompress_jpeg_image_from_file( char const*pSrc_filename, int *width, int *height, int *actual_comps, int req_comps);


} // namespace jpgd

#endif // JPEG_DECODER_H

