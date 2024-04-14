#pragma once

#include <Drawing.h>

#pragma pack(4)
struct IsoTileImageStruct
{
	signed int  XPos;
	signed int  YPos;
	signed int  ExtraOffset;
	signed int  ZDataOffset;
	signed int  ExtraZOffset;
	signed int  ExtraXPos;
	signed int  ExtraYPos;
	signed int  ExtraWidth;
	signed int  ExtraHeight;
	signed int  Flags;
	unsigned char Height;
	unsigned char TileType;
	unsigned char RampType;
	ColorStruct LowColor;
	ColorStruct HighColor;
	unsigned char field_31[3];
};
#pragma pack()

#pragma pack(4)
struct IsoTileHeaderStruct
{
	signed int  Width;
	signed int  Height;
	signed int  ImageWidth;
	signed int  ImageHeight;
};
#pragma pack()

//#pragma pack(4)
//struct IsoTileFileStruct
//{
//public:
//	operator void* () const { return (*this); } // This allows the struct to be passed implicitly as a raw pointer.
//
//	IsoTileImageStruct* Get_Tiles_Data(int index)
//	{
//		return &(&Tiles)[index * sizeof(IsoTileImageStruct)];
//	}
//
//	int Get_Width() const { return Header.Width; }
//	int Get_Height() const { return Header.Height; }
//
//private:
//	IsoTileHeaderStruct Header;
//
//	/**
//	 *  This is an instance of the first frame in the iso tile file, use Get_Frame_Data
//	 *  to get the image information, do not access this directly!
//	 */
//	IsoTileImageStruct Tiles;
//};
//#pragma pack()
