#pragma once

#include <CCINIClass.h>
#include <Point2D.h>
#include <Point3D.h>
#include <RectangleStruct.h>
#include <string>

class FileStraw;
/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class INIClassExt : public INIClass
{
public:
	int _Load(FileStraw* ffile, bool loadcomments);
	int _Get_String(char const* section, char const* entry, char const* defvalue, char* buffer, int size) const;

	int _Get_Int(char const* section, char const* entry, int defvalue) const;
	bool _Get_Bool(char const* section, char const* entry, bool defvalue) const;
	double _Get_Float(char const* section, char const* entry, double defvalue) const;
	Point2D _Get_Point(char const* section, char const* entry, Point2D const& defvalue) const;
	Point3D _Get_Point(char const* section, char const* entry, Point3D const& defvalue) const;
	Vector3D<float> _Get_Point(char const* section, char const* entry, Vector3D<float> const& defvalue) const;
	int _Get_UUBlock(char const* section, void* block, int len) const;
	int _Get_TextBlock(char const* section, char* buffer, int len) const;
	CLSID _Get_UUID(char const* section, char const* entry, CLSID defvalue) const;
	RectangleStruct _Get_RectangleStruct(char const* section, char const* entry, RectangleStruct const& defvalue) const;

	void Inherit_File(INIClass const& ini);
	void Include_File(INIClass const& ini);

	int ReadString(const char* pSection, const char* pKey, const char* pDefault, char* pBuffer, size_t szBufferSize)
	{
		JMP_THIS(0x528A10);
	}

	std::string Get_String(char const* section, char const* entry, std::string const& defvalue) ;
	std::string Get_TextBlock(char const* section);
};
