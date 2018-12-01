#include <stdio.h>
#include <vector>

/*
decodePNG: The picoPNG function, decodes a PNG file buffer in memory, into a raw pixel buffer.
out_image: output parameter, this will contain the raw pixels after decoding.
  By default the output is 32-bit RGBA color.
  The std::vector is automatically resized to the correct size.
image_width: output_parameter, this will contain the width of the image in pixels.
image_height: output_parameter, this will contain the height of the image in pixels.
in_png: pointer to the buffer of the PNG file in memory. To get it from a file on
  disk, load it and store it in a memory buffer yourself first.
in_size: size of the input PNG file in bytes.
convert_to_rgba32: optional parameter, true by default.
  Set to true to get the output in RGBA 32-bit (8 bit per channel) color format
  no matter what color type the original PNG image had. This gives predictable,
  useable data from any random input PNG.
  Set to false to do no color conversion at all. The result then has the same data
  type as the PNG image, which can range from 1 bit to 64 bits per pixel.
  Information about the color type or palette colors are not provided. You need
  to know this information yourself to be able to use the data so this only
  works for trusted PNG files. Use LodePNG instead of picoPNG if you need this information.
return: 0 if success, not 0 if some error occured.
*/
int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32)
{
  // picoPNG version 20101224
  // Copyright (c) 2005-2010 Lode Vandevenne
  //
  // This software is provided 'as-is', without any express or implied
  // warranty. In no event will the authors be held liable for any damages
  // arising from the use of this software.
  //
  // Permission is granted to anyone to use this software for any purpose,
  // including commercial applications, and to alter it and redistribute it
  // freely, subject to the following restrictions:
  //
  //     1. The origin of this software must not be misrepresented; you must not
  //     claim that you wrote the original software. If you use this software
  //     in a product, an acknowledgment in the product documentation would be
  //     appreciated but is not required.
  //     2. Altered source versions must be plainly marked as such, and must not be
  //     misrepresented as being the original software.
  //     3. This notice may not be removed or altered from any source distribution.
  
  // picoPNG is a PNG decoder in one C++ function of around 500 lines. Use picoPNG for
  // programs that need only 1 .cpp file. Since it's a single function, it's very limited,
  // it can convert a PNG to raw pixel data either converted to 32-bit RGBA color or
  // with no color conversion at all. For anything more complex, another tiny library
  // is available: LodePNG (lodepng.c(pp)), which is a single source and header file.
  // Apologies for the compact code style, it's to make this tiny.
  
  static const unsigned long LENBASE[29] =  {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
  static const unsigned long LENEXTRA[29] = {0,0,0,0,0,0,0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5,  0};
  static const unsigned long DISTBASE[30] =  {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
  static const unsigned long DISTEXTRA[30] = {0,0,0,0,1,1,2, 2, 3, 3, 4, 4, 5, 5,  6,  6,  7,  7,  8,  8,   9,   9,  10,  10,  11,  11,  12,   12,   13,   13};
  static const unsigned long CLCL[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15}; //code length code lengths
  struct Zlib //nested functions for zlib decompression
  {
    static unsigned long readBitFromStream(size_t& bitp, const unsigned char* bits) { unsigned long result = (bits[bitp >> 3] >> (bitp & 0x7)) & 1; bitp++; return result;}
    static unsigned long readBitsFromStream(size_t& bitp, const unsigned char* bits, size_t nbits)
    {
      unsigned long result = 0;
      for(size_t i = 0; i < nbits; i++) result += (readBitFromStream(bitp, bits)) << i;
      return result;
    }
    struct HuffmanTree
    {
      int makeFromLengths(const std::vector<unsigned long>& bitlen, unsigned long maxbitlen)
      { //make tree given the lengths
        unsigned long numcodes = (unsigned long)(bitlen.size()), treepos = 0, nodefilled = 0;
        std::vector<unsigned long> tree1d(numcodes), blcount(maxbitlen + 1, 0), nextcode(maxbitlen + 1, 0);
        for(unsigned long bits = 0; bits < numcodes; bits++) blcount[bitlen[bits]]++; //count number of instances of each code length
        for(unsigned long bits = 1; bits <= maxbitlen; bits++) nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;
        for(unsigned long n = 0; n < numcodes; n++) if(bitlen[n] != 0) tree1d[n] = nextcode[bitlen[n]]++; //generate all the codes
        tree2d.clear(); tree2d.resize(numcodes * 2, 32767); //32767 here means the tree2d isn't filled there yet
        for(unsigned long n = 0; n < numcodes; n++) //the codes
        for(unsigned long i = 0; i < bitlen[n]; i++) //the bits for this code
        {
          unsigned long bit = (tree1d[n] >> (bitlen[n] - i - 1)) & 1;
          if(treepos > numcodes - 2) return 55;
          if(tree2d[2 * treepos + bit] == 32767) //not yet filled in
          {
            if(i + 1 == bitlen[n]) { tree2d[2 * treepos + bit] = n; treepos = 0; } //last bit
            else { tree2d[2 * treepos + bit] = ++nodefilled + numcodes; treepos = nodefilled; } //addresses are encoded as values > numcodes
          }
          else treepos = tree2d[2 * treepos + bit] - numcodes; //subtract numcodes from address to get address value
        }
        return 0;
      }
      int decode(bool& decoded, unsigned long& result, size_t& treepos, unsigned long bit) const
      { //Decodes a symbol from the tree
        unsigned long numcodes = (unsigned long)tree2d.size() / 2;
        if(treepos >= numcodes) return 11; //error: you appeared outside the codetree
        result = tree2d[2 * treepos + bit];
        decoded = (result < numcodes);
        treepos = decoded ? 0 : result - numcodes;
        return 0;
      }
      std::vector<unsigned long> tree2d; //2D representation of a huffman tree: The one dimension is "0" or "1", the other contains all nodes and leaves of the tree.
    };
    struct Inflator
    {
      int error;
      void inflate(std::vector<unsigned char>& out, const std::vector<unsigned char>& iString, size_t inpos = 0)
      {
        size_t bp = 0, pos = 0; //bit pointer and byte pointer
        error = 0;
        unsigned long BFINAL = 0;
        while(!BFINAL && !error)
        {
          if(bp >> 3 >= iString.size()) { error = 52; return; } //error, bit pointer will jump past memory
          BFINAL = readBitFromStream(bp, &iString[inpos]);
          unsigned long BTYPE = readBitFromStream(bp, &iString[inpos]); BTYPE += 2 * readBitFromStream(bp, &iString[inpos]);
          if(BTYPE == 3) { error = 20; return; } //error: invalid BTYPE
          else if(BTYPE == 0) inflateNoCompression(out, &iString[inpos], bp, pos, iString.size());
          else inflateHuffmanBlock(out, &iString[inpos], bp, pos, iString.size(), BTYPE);
        }
        if(!error) out.resize(pos); //Only now we know the true size of out, resize it to that
      }
      void generateFixedTrees(HuffmanTree& tree, HuffmanTree& treeD) //get the tree of a deflated block with fixed tree
      {
        std::vector<unsigned long> bitlen(288, 8), bitlenD(32, 5);;
        for(size_t i = 144; i <= 255; i++) bitlen[i] = 9;
        for(size_t i = 256; i <= 279; i++) bitlen[i] = 7;
        tree.makeFromLengths(bitlen, 15);
        treeD.makeFromLengths(bitlenD, 15);
      }
      HuffmanTree codetree, codetreeD, codelengthcodetree; //the code tree for Huffman codes, dist codes, and code length codes
      unsigned long huffmanDecodeSymbol(const unsigned char* iString, size_t& bp, const HuffmanTree& codetree, size_t inlength)
      { //decode a single symbol from given list of bits with given code tree. return value is the symbol
        bool decoded; unsigned long ct;
        for(size_t treepos = 0;;)
        {
          if((bp & 0x07) == 0 && (bp >> 3) > inlength) { error = 10; return 0; } //error: end reached without endcode
          error = codetree.decode(decoded, ct, treepos, readBitFromStream(bp, iString)); if(error) return 0; //stop, an error happened
          if(decoded) return ct;
        }
      }
      void getTreeInflateDynamic(HuffmanTree& tree, HuffmanTree& treeD, const unsigned char* iString, size_t& bp, size_t inlength)
      { //get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree
        std::vector<unsigned long> bitlen(288, 0), bitlenD(32, 0);
        if(bp >> 3 >= inlength - 2) { error = 49; return; } //the bit pointer is or will go past the memory
        size_t HLIT =  readBitsFromStream(bp, iString, 5) + 257; //number of literal/length codes + 257
        size_t HDIST = readBitsFromStream(bp, iString, 5) + 1; //number of dist codes + 1
        size_t HCLEN = readBitsFromStream(bp, iString, 4) + 4; //number of code length codes + 4
        std::vector<unsigned long> codelengthcode(19); //lengths of tree to decode the lengths of the dynamic tree
        for(size_t i = 0; i < 19; i++) codelengthcode[CLCL[i]] = (i < HCLEN) ? readBitsFromStream(bp, iString, 3) : 0;
        error = codelengthcodetree.makeFromLengths(codelengthcode, 7); if(error) return;
        size_t i = 0, replength;
        while(i < HLIT + HDIST)
        {
          unsigned long code = huffmanDecodeSymbol(iString, bp, codelengthcodetree, inlength); if(error) return;
          if(code <= 15)  { if(i < HLIT) bitlen[i++] = code; else bitlenD[i++ - HLIT] = code; } //a length code
          else if(code == 16) //repeat previous
          {
            if(bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
            replength = 3 + readBitsFromStream(bp, iString, 2);
            unsigned long value; //set value to the previous code
            if((i - 1) < HLIT) value = bitlen[i - 1];
            else value = bitlenD[i - HLIT - 1];
            for(size_t n = 0; n < replength; n++) //repeat this value in the next lengths
            {
              if(i >= HLIT + HDIST) { error = 13; return; } //error: i is larger than the amount of codes
              if(i < HLIT) bitlen[i++] = value; else bitlenD[i++ - HLIT] = value;
            }
          }
          else if(code == 17) //repeat "0" 3-10 times
          {
            if(bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
            replength = 3 + readBitsFromStream(bp, iString, 3);
            for(size_t n = 0; n < replength; n++) //repeat this value in the next lengths
            {
              if(i >= HLIT + HDIST) { error = 14; return; } //error: i is larger than the amount of codes
              if(i < HLIT) bitlen[i++] = 0; else bitlenD[i++ - HLIT] = 0;
            }
          }
          else if(code == 18) //repeat "0" 11-138 times
          {
            if(bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
            replength = 11 + readBitsFromStream(bp, iString, 7);
            for(size_t n = 0; n < replength; n++) //repeat this value in the next lengths
            {
              if(i >= HLIT + HDIST) { error = 15; return; } //error: i is larger than the amount of codes
              if(i < HLIT) bitlen[i++] = 0; else bitlenD[i++ - HLIT] = 0;
            }
          }
          else { error = 16; return; } //error: somehow an unexisting code appeared. This can never happen.
        }
        if(bitlen[256] == 0) { error = 64; return; } //the length of the end code 256 must be larger than 0
        error = tree.makeFromLengths(bitlen, 15); if(error) return; //now we've finally got HLIT and HDIST, so generate the code trees, and the function is done
        error = treeD.makeFromLengths(bitlenD, 15); if(error) return;
      }
      void inflateHuffmanBlock(std::vector<unsigned char>& out, const unsigned char* iString, size_t& bp, size_t& pos, size_t inlength, unsigned long btype) 
      {
        if(btype == 1) { generateFixedTrees(codetree, codetreeD); }
        else if(btype == 2) { getTreeInflateDynamic(codetree, codetreeD, iString, bp, inlength); if(error) return; }
        for(;;)
        {
          unsigned long code = huffmanDecodeSymbol(iString, bp, codetree, inlength); if(error) return;
          if(code == 256) return; //end code
          else if(code <= 255) //literal symbol
          {
            if(pos >= out.size()) out.resize((pos + 1) * 2); //reserve more room
            out[pos++] = (unsigned char)(code);
          }
          else if(code >= 257 && code <= 285) //length code
          {
            size_t length = LENBASE[code - 257], numextrabits = LENEXTRA[code - 257];
            if((bp >> 3) >= inlength) { error = 51; return; } //error, bit pointer will jump past memory
            length += readBitsFromStream(bp, iString, numextrabits);
            unsigned long codeD = huffmanDecodeSymbol(iString, bp, codetreeD, inlength); if(error) return;
            if(codeD > 29) { error = 18; return; } //error: invalid dist code (30-31 are never used)
            unsigned long dist = DISTBASE[codeD], numextrabitsD = DISTEXTRA[codeD];
            if((bp >> 3) >= inlength) { error = 51; return; } //error, bit pointer will jump past memory
            dist += readBitsFromStream(bp, iString, numextrabitsD);
            size_t start = pos, back = start - dist; //backwards
            if(pos + length >= out.size()) out.resize((pos + length) * 2); //reserve more room
            for(size_t i = 0; i < length; i++) { out[pos++] = out[back++]; if(back >= start) back = start - dist; }
          }
        }
      }
      void inflateNoCompression(std::vector<unsigned char>& out, const unsigned char* iString, size_t& bp, size_t& pos, size_t inlength)
      {
        while((bp & 0x7) != 0) bp++; //go to first boundary of byte
        size_t p = bp / 8;
        if(p >= inlength - 4) { error = 52; return; } //error, bit pointer will jump past memory
        unsigned long LEN = iString[p] + 256 * iString[p + 1], NLEN = iString[p + 2] + 256 * iString[p + 3]; p += 4;
        if(LEN + NLEN != 65535) { error = 21; return; } //error: NLEN is not one's complement of LEN
        if(pos + LEN >= out.size()) out.resize(pos + LEN);
        if(p + LEN > inlength) { error = 23; return; } //error: reading outside of in buffer
        for(unsigned long n = 0; n < LEN; n++) out[pos++] = iString[p++]; //read LEN bytes of literal data
        bp = p * 8;
      }
    };
    int decompress(std::vector<unsigned char>& out, const std::vector<unsigned char>& iString) //returns error value
    {
      Inflator inflator;
      if(iString.size() < 2) { return 53; } //error, size of zlib data too small
      if((iString[0] * 256 + iString[1]) % 31 != 0) { return 24; } //error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way
      unsigned long CM = iString[0] & 15, CINFO = (iString[0] >> 4) & 15, FDICT = (iString[1] >> 5) & 1;
      if(CM != 8 || CINFO > 7) { return 25; } //error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec
      if(FDICT != 0) { return 26; } //error: the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset dictionary."
      inflator.inflate(out, iString, 2);
      return inflator.error; //note: adler32 checksum was skipped and ignored
    }
  };
  struct PNG //nested functions for PNG decoding
  {
    struct Info
    {
      unsigned long width, height, colorType, bitDepth, compressionMethod, filterMethod, interlaceMethod, key_r, key_g, key_b;
      bool key_defined; //is a transparent color key given?
      std::vector<unsigned char> palette;
    } info;
    int error;
    void decode(std::vector<unsigned char>& out, const unsigned char* iString, size_t size, bool convert_to_rgba32)
    {
      error = 0;
      if(size == 0 || iString == 0) { error = 48; return; } //the given data is empty
      readPngHeader(&iString[0], size); if(error) return;
      size_t pos = 33; //first byte of the first chunk after the header
      std::vector<unsigned char> idat; //the data from idat chunks
      bool IEND = false, known_type = true;
      info.key_defined = false;
      while(!IEND) //loop through the chunks, ignoring unknown chunks and stopping at IEND chunk. IDAT data is put at the start of the in buffer
      {
        if(pos + 8 >= size) { error = 30; return; } //error: size of the in buffer too small to contain next chunk
        size_t chunkLength = read32bitInt(&iString[pos]); pos += 4;
        if(chunkLength > 2147483647) { error = 63; return; }
        if(pos + chunkLength >= size) { error = 35; return; } //error: size of the in buffer too small to contain next chunk
        if(iString[pos + 0] == 'I' && iString[pos + 1] == 'D' && iString[pos + 2] == 'A' && iString[pos + 3] == 'T') //IDAT chunk, containing compressed image data
        {
          idat.insert(idat.end(), &iString[pos + 4], &iString[pos + 4 + chunkLength]);
          pos += (4 + chunkLength);
        }
        else if(iString[pos + 0] == 'I' && iString[pos + 1] == 'E' && iString[pos + 2] == 'N' && iString[pos + 3] == 'D')  { pos += 4; IEND = true; }
        else if(iString[pos + 0] == 'P' && iString[pos + 1] == 'L' && iString[pos + 2] == 'T' && iString[pos + 3] == 'E') //palette chunk (PLTE)
        {
          pos += 4; //go after the 4 letters
          info.palette.resize(4 * (chunkLength / 3));
          if(info.palette.size() > (4 * 256)) { error = 38; return; } //error: palette too big
          for(size_t i = 0; i < info.palette.size(); i += 4)
          {
            for(size_t j = 0; j < 3; j++) info.palette[i + j] = iString[pos++]; //RGB
            info.palette[i + 3] = 255; //alpha
          }
        }
        else if(iString[pos + 0] == 't' && iString[pos + 1] == 'R' && iString[pos + 2] == 'N' && iString[pos + 3] == 'S') //palette transparency chunk (tRNS)
        {
          pos += 4; //go after the 4 letters
          if(info.colorType == 3)
          {
            if(4 * chunkLength > info.palette.size()) { error = 39; return; } //error: more alpha values given than there are palette entries
            for(size_t i = 0; i < chunkLength; i++) info.palette[4 * i + 3] = iString[pos++];
          }
          else if(info.colorType == 0)
          {
            if(chunkLength != 2) { error = 40; return; } //error: this chunk must be 2 bytes for greyscale image
            info.key_defined = 1; info.key_r = info.key_g = info.key_b = 256 * iString[pos] + iString[pos + 1]; pos += 2;
          }
          else if(info.colorType == 2)
          {
            if(chunkLength != 6) { error = 41; return; } //error: this chunk must be 6 bytes for RGB image
            info.key_defined = 1;
            info.key_r = 256 * iString[pos] + iString[pos + 1]; pos += 2;
            info.key_g = 256 * iString[pos] + iString[pos + 1]; pos += 2;
            info.key_b = 256 * iString[pos] + iString[pos + 1]; pos += 2;
          }
          else { error = 42; return; } //error: tRNS chunk not allowed for other color models
        }
        else //it's not an implemented chunk type, so ignore it: skip over the data
        {
          if(!(iString[pos + 0] & 32)) { error = 69; return; } //error: unknown critical chunk (5th bit of first byte of chunk type is 0)
          pos += (chunkLength + 4); //skip 4 letters and uninterpreted data of unimplemented chunk
          known_type = false;
        }
        pos += 4; //step over CRC (which is ignored)
      }
      unsigned long bpp = getBpp(info);
      std::vector<unsigned char> scanlines(((info.width * (info.height * bpp + 7)) / 8) + info.height); //now the out buffer will be filled
      Zlib zlib; //decompress with the Zlib decompressor
      error = zlib.decompress(scanlines, idat); if(error) return; //stop if the zlib decompressor returned an error
      size_t bytewidth = (bpp + 7) / 8, outlength = (info.height * info.width * bpp + 7) / 8;
      out.resize(outlength); //time to fill the out buffer
      unsigned char* out_ = outlength ? &out[0] : 0; //use a regular pointer to the std::vector for faster code if compiled without optimization
      if(info.interlaceMethod == 0) //no interlace, just filter
      {
        size_t linestart = 0, linelength = (info.width * bpp + 7) / 8; //length in bytes of a scanline, excluding the filtertype byte
        if(bpp >= 8) //byte per byte
        for(unsigned long y = 0; y < info.height; y++)
        {
          unsigned long filterType = scanlines[linestart];
          const unsigned char* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
          unFilterScanline(&out_[linestart - y], &scanlines[linestart + 1], prevline, bytewidth, filterType,  linelength); if(error) return;
          linestart += (1 + linelength); //go to start of next scanline
        }
        else //less than 8 bits per pixel, so fill it up bit per bit
        {
          std::vector<unsigned char> templine((info.width * bpp + 7) >> 3); //only used if bpp < 8
          for(size_t y = 0, obp = 0; y < info.height; y++)
          {
            unsigned long filterType = scanlines[linestart];
            const unsigned char* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
            unFilterScanline(&templine[0], &scanlines[linestart + 1], prevline, bytewidth, filterType, linelength); if(error) return;
            for(size_t bp = 0; bp < info.width * bpp;) setBitOfReversedStream(obp, out_, readBitFromReversedStream(bp, &templine[0]));
            linestart += (1 + linelength); //go to start of next scanline
          }
        }
      }
      else //interlaceMethod is 1 (Adam7)
      {
        size_t passw[7] = { (info.width + 7) / 8, (info.width + 3) / 8, (info.width + 3) / 4, (info.width + 1) / 4, (info.width + 1) / 2, (info.width + 0) / 2, (info.width + 0) / 1 };
        size_t passh[7] = { (info.height + 7) / 8, (info.height + 7) / 8, (info.height + 3) / 8, (info.height + 3) / 4, (info.height + 1) / 4, (info.height + 1) / 2, (info.height + 0) / 2 };
        size_t passstart[7] = {0};
        size_t pattern[28] = {0,4,0,2,0,1,0,0,0,4,0,2,0,1,8,8,4,4,2,2,1,8,8,8,4,4,2,2}; //values for the adam7 passes
        for(int i = 0; i < 6; i++) passstart[i + 1] = passstart[i] + passh[i] * ((passw[i] ? 1 : 0) + (passw[i] * bpp + 7) / 8);
        std::vector<unsigned char> scanlineo((info.width * bpp + 7) / 8), scanlinen((info.width * bpp + 7) / 8); //"old" and "new" scanline
        for(int i = 0; i < 7; i++)
          adam7Pass(&out_[0], &scanlinen[0], &scanlineo[0], &scanlines[passstart[i]], info.width, pattern[i], pattern[i + 7], pattern[i + 14], pattern[i + 21], passw[i], passh[i], bpp);
      }
      if(convert_to_rgba32 && (info.colorType != 6 || info.bitDepth != 8)) //conversion needed
      {
        std::vector<unsigned char> data = out;
        error = convert(out, &data[0], info, info.width, info.height);
      }
    }
    void readPngHeader(const unsigned char* iString, size_t inlength) //read the information from the header and store it in the Info
    {
      if(inlength < 29) { error = 27; return; } //error: the data length is smaller than the length of the header
      if(iString[0] != 137 || iString[1] != 80 || iString[2] != 78 || iString[3] != 71 || iString[4] != 13 || iString[5] != 10 || iString[6] != 26 || iString[7] != 10) { error = 28; return; } //no PNG signature
      if(iString[12] != 'I' || iString[13] != 'H' || iString[14] != 'D' || iString[15] != 'R') { error = 29; return; } //error: it doesn't start with a IHDR chunk!
      info.width = read32bitInt(&iString[16]); info.height = read32bitInt(&iString[20]);
      info.bitDepth = iString[24]; info.colorType = iString[25];
      info.compressionMethod = iString[26]; if(iString[26] != 0) { error = 32; return; } //error: only compression method 0 is allowed in the specification
      info.filterMethod = iString[27]; if(iString[27] != 0) { error = 33; return; } //error: only filter method 0 is allowed in the specification
      info.interlaceMethod = iString[28]; if(iString[28] > 1) { error = 34; return; } //error: only interlace methods 0 and 1 exist in the specification
      error = checkColorValidity(info.colorType, info.bitDepth);
    }
    void unFilterScanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon, size_t bytewidth, unsigned long filterType, size_t length)
    {
      switch(filterType)
      {
        case 0: for(size_t i = 0; i < length; i++) recon[i] = scanline[i]; break;
        case 1:
          for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i];
          for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + recon[i - bytewidth];
          break;
        case 2:
          if(precon) for(size_t i = 0; i < length; i++) recon[i] = scanline[i] + precon[i];
          else       for(size_t i = 0; i < length; i++) recon[i] = scanline[i];
          break;
        case 3:
          if(precon)
          {
            for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i] + precon[i] / 2;
            for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
          }
          else
          {
            for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i];
            for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + recon[i - bytewidth] / 2;
          }
          break;
        case 4:
          if(precon)
          {
            for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i] + paethPredictor(0, precon[i], 0);
            for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]);
          }
          else
          {
            for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i];
            for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + paethPredictor(recon[i - bytewidth], 0, 0);
          }
          break;
        default: error = 36; return; //error: unexisting filter type given
      }
    }
    void adam7Pass(unsigned char* out, unsigned char* linen, unsigned char* lineo, const unsigned char* iString, unsigned long w, size_t passleft, size_t passtop, size_t spacex, size_t spacey, size_t passw, size_t passh, unsigned long bpp)
    { //filter and reposition the pixels into the output when the image is Adam7 interlaced. This function can only do it after the full image is already decoded. The out buffer must have the correct allocated memory size already.
      if(passw == 0) return;
      size_t bytewidth = (bpp + 7) / 8, linelength = 1 + ((bpp * passw + 7) / 8);
      for(unsigned long y = 0; y < passh; y++)
      {
        unsigned char filterType = iString[y * linelength], *prevline = (y == 0) ? 0 : lineo;
        unFilterScanline(linen, &iString[y * linelength + 1], prevline, bytewidth, filterType, (w * bpp + 7) / 8); if(error) return;
        if(bpp >= 8) for(size_t i = 0; i < passw; i++) for(size_t b = 0; b < bytewidth; b++) //b = current byte of this pixel
          out[bytewidth * w * (passtop + spacey * y) + bytewidth * (passleft + spacex * i) + b] = linen[bytewidth * i + b];
        else for(size_t i = 0; i < passw; i++)
        {
          size_t obp = bpp * w * (passtop + spacey * y) + bpp * (passleft + spacex * i), bp = i * bpp;
          for(size_t b = 0; b < bpp; b++) setBitOfReversedStream(obp, out, readBitFromReversedStream(bp, &linen[0]));
        }
        unsigned char* temp = linen; linen = lineo; lineo = temp; //swap the two buffer pointers "line old" and "line new"
      }
    }
    static unsigned long readBitFromReversedStream(size_t& bitp, const unsigned char* bits) { unsigned long result = (bits[bitp >> 3] >> (7 - (bitp & 0x7))) & 1; bitp++; return result;}
    static unsigned long readBitsFromReversedStream(size_t& bitp, const unsigned char* bits, unsigned long nbits)
    {
      unsigned long result = 0;
      for(size_t i = nbits - 1; i < nbits; i--) result += ((readBitFromReversedStream(bitp, bits)) << i);
      return result;
    }
    void setBitOfReversedStream(size_t& bitp, unsigned char* bits, unsigned long bit) { bits[bitp >> 3] |=  (bit << (7 - (bitp & 0x7))); bitp++; }
    unsigned long read32bitInt(const unsigned char* buffer) { return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]; }
    int checkColorValidity(unsigned long colorType, unsigned long bd) //return type is a LodePNG error code
    {
      if((colorType == 2 || colorType == 4 || colorType == 6)) { if(!(bd == 8 || bd == 16)) return 37; else return 0; }
      else if(colorType == 0) { if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37; else return 0; }
      else if(colorType == 3) { if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8            )) return 37; else return 0; }
      else return 31; //unexisting color type
    }
    unsigned long getBpp(const Info& info)
    {
      if(info.colorType == 2) return (3 * info.bitDepth);
      else if(info.colorType >= 4) return (info.colorType - 2) * info.bitDepth;
      else return info.bitDepth;
    }
    int convert(std::vector<unsigned char>& out, const unsigned char* iString, Info& infoIn, unsigned long w, unsigned long h)
    { //converts from any color type to 32-bit. return value = LodePNG error code
      size_t numpixels = w * h, bp = 0;
      out.resize(numpixels * 4);
      unsigned char* out_ = out.empty() ? 0 : &out[0]; //faster if compiled without optimization
      if(infoIn.bitDepth == 8 && infoIn.colorType == 0) //greyscale
      for(size_t i = 0; i < numpixels; i++)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = iString[i];
        out_[4 * i + 3] = (infoIn.key_defined && iString[i] == infoIn.key_r) ? 0 : 255;
      }
      else if(infoIn.bitDepth == 8 && infoIn.colorType == 2) //RGB color
      for(size_t i = 0; i < numpixels; i++)
      {
        for(size_t c = 0; c < 3; c++) out_[4 * i + c] = iString[3 * i + c];
        out_[4 * i + 3] = (infoIn.key_defined == 1 && iString[3 * i + 0] == infoIn.key_r && iString[3 * i + 1] == infoIn.key_g && iString[3 * i + 2] == infoIn.key_b) ? 0 : 255;
      }
      else if(infoIn.bitDepth == 8 && infoIn.colorType == 3) //indexed color (palette)
      for(size_t i = 0; i < numpixels; i++)
      {
        if(4U * iString[i] >= infoIn.palette.size()) return 46;
        for(size_t c = 0; c < 4; c++) out_[4 * i + c] = infoIn.palette[4 * iString[i] + c]; //get rgb colors from the palette
      }
      else if(infoIn.bitDepth == 8 && infoIn.colorType == 4) //greyscale with alpha
      for(size_t i = 0; i < numpixels; i++)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = iString[2 * i + 0];
        out_[4 * i + 3] = iString[2 * i + 1];
      }
      else if(infoIn.bitDepth == 8 && infoIn.colorType == 6) for(size_t i = 0; i < numpixels; i++) for(size_t c = 0; c < 4; c++) out_[4 * i + c] = iString[4 * i + c]; //RGB with alpha
      else if(infoIn.bitDepth == 16 && infoIn.colorType == 0) //greyscale
      for(size_t i = 0; i < numpixels; i++)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = iString[2 * i];
        out_[4 * i + 3] = (infoIn.key_defined && 256U * iString[i] + iString[i + 1] == infoIn.key_r) ? 0 : 255;
      }
      else if(infoIn.bitDepth == 16 && infoIn.colorType == 2) //RGB color
      for(size_t i = 0; i < numpixels; i++)
      {
        for(size_t c = 0; c < 3; c++) out_[4 * i + c] = iString[6 * i + 2 * c];
        out_[4 * i + 3] = (infoIn.key_defined && 256U*iString[6*i+0]+iString[6*i+1] == infoIn.key_r && 256U*iString[6*i+2]+iString[6*i+3] == infoIn.key_g && 256U*iString[6*i+4]+iString[6*i+5] == infoIn.key_b) ? 0 : 255;
      }
      else if(infoIn.bitDepth == 16 && infoIn.colorType == 4) //greyscale with alpha
      for(size_t i = 0; i < numpixels; i++)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = iString[4 * i]; //most significant byte
        out_[4 * i + 3] = iString[4 * i + 2];
      }
      else if(infoIn.bitDepth == 16 && infoIn.colorType == 6) for(size_t i = 0; i < numpixels; i++) for(size_t c = 0; c < 4; c++) out_[4 * i + c] = iString[8 * i + 2 * c]; //RGB with alpha
      else if(infoIn.bitDepth < 8 && infoIn.colorType == 0) //greyscale
      for(size_t i = 0; i < numpixels; i++)
      {
        unsigned long value = (readBitsFromReversedStream(bp, iString, infoIn.bitDepth) * 255) / ((1 << infoIn.bitDepth) - 1); //scale value from 0 to 255
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = (unsigned char)(value);
        out_[4 * i + 3] = (infoIn.key_defined && value && ((1U << infoIn.bitDepth) - 1U) == infoIn.key_r && ((1U << infoIn.bitDepth) - 1U)) ? 0 : 255;
      }
      else if(infoIn.bitDepth < 8 && infoIn.colorType == 3) //palette
      for(size_t i = 0; i < numpixels; i++)
      {
        unsigned long value = readBitsFromReversedStream(bp, iString, infoIn.bitDepth);
        if(4 * value >= infoIn.palette.size()) return 47;
        for(size_t c = 0; c < 4; c++) out_[4 * i + c] = infoIn.palette[4 * value + c]; //get rgb colors from the palette
      }
      return 0;
    }
    unsigned char paethPredictor(short a, short b, short c) //Paeth predicter, used by PNG filter type 4
    {
      short p = a + b - c, pa = p > a ? (p - a) : (a - p), pb = p > b ? (p - b) : (b - p), pc = p > c ? (p - c) : (c - p);
      return (unsigned char)((pa <= pb && pa <= pc) ? a : pb <= pc ? b : c);
    }
  };
  PNG decoder; decoder.decode(out_image, in_png, in_size, convert_to_rgba32);
  image_width = decoder.info.width; image_height = decoder.info.height;
  return decoder.error;
}





//an example using the PNG loading function:

#include <iostream>
#include <fstream>

void loadFile(std::vector<unsigned char>& buffer, const std::string& filename) //designed for loading files from hard disk in an std::vector
{
  std::ifstream file(filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

  //get filesize
  std::streamsize size = 0;
  if(file.seekg(0, std::ios::end).good()) size = file.tellg();
  if(file.seekg(0, std::ios::beg).good()) size -= file.tellg();

  //read contents of the file into the vector
  if(size > 0)
  {
    buffer.resize((size_t)size);
    file.read((char*)(&buffer[0]), size);
  }
  else buffer.clear();
}

