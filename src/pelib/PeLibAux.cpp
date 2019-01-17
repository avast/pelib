/*
* PeLibAux.cpp - Part of the PeLib library.
*
* Copyright (c) 2004 - 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
* or the license information file (license.htm) in the root directory
* of PeLib.
*/

#include <vector>

#ifdef _MSC_VER
  #include <ctype.h>
#endif

#include "pelib/PeLibInc.h"
#include "pelib/PeLibAux.h"
#include "pelib/PeFile.h"

namespace PeLib
{
	const qword PELIB_IMAGE_ORDINAL_FLAGS<64>::PELIB_IMAGE_ORDINAL_FLAG = 0x8000000000000000ULL;

	// Keep in sync with PeLib::LoaderError!!!
	static const std::vector<LoaderErrorString> LdrErrStrings =
	{
		{"LDR_ERROR_NONE",                         "No error"},
		{"LDR_ERROR_FILE_TOO_BIG",                 "The file is larger than 4GB - 1"},
		{"LDR_ERROR_E_LFANEW_UNALIGNED",           "The IMAGE_DOS_HEADER::e_lfanew is not aligned to 4"},
		{"LDR_ERROR_E_LFANEW_OUT_OF_FILE",         "The IMAGE_DOS_HEADER::e_lfanew is out of (lower 4 GB of) the file"},
		{"LDR_ERROR_NTHEADER_OFFSET_OVERFLOW",     "NT header offset + sizeof(IMAGE_NT_HEADERS) overflow"},
		{"LDR_ERROR_NTHEADER_OUT_OF_FILE",         "NT header offset + sizeof(IMAGE_NT_HEADERS) is greater than filesize"},
		{"LDR_ERROR_NO_NT_SIGNATURE",              "Missing IMAGE_NT_SIGNATURE in the NT headers" },
		{"LDR_ERROR_FILE_HEADER_INVALID",          "Invalid IMAGE_FILE_HEADER::Machine or IMAGE_FILE_HEADER::SizeOfOptionalHeader" },
		{"LDR_ERROR_IMAGE_NON_EXECUTABLE",         "Missing IMAGE_FILE_EXECUTABLE_IMAGE in IMAGE_FILE_HEADER::Characteristics" },
		{"LDR_ERROR_NO_OPTHDR_MAGIC",              "Invalid IMAGE_OPTIONAL_HEADER::Magic" },
		{"LDR_ERROR_SIZE_OF_HEADERS_ZERO",         "IMAGE_OPTIONAL_HEADER::SizeOfHeaders is zero" },
		{"LDR_ERROR_FILE_ALIGNMENT_ZERO",          "IMAGE_OPTIONAL_HEADER::FileAlignment is zero" },
		{"LDR_ERROR_FILE_ALIGNMENT_NOT_POW2",      "IMAGE_OPTIONAL_HEADER::FileAlignment is not power of two" },
		{"LDR_ERROR_SECTION_ALIGNMENT_ZERO",       "IMAGE_OPTIONAL_HEADER::SectionAlignment is zero" },
		{"LDR_ERROR_SECTION_ALIGNMENT_NOT_POW2",   "IMAGE_OPTIONAL_HEADER::SectionAlignment is not power of two" },
		{"LDR_ERROR_SECTION_ALIGNMENT_TOO_SMALL",  "IMAGE_OPTIONAL_HEADER::SectionAlignment is smaller than IMAGE_OPTIONAL_HEADER::FileAlignment" },
		{"LDR_ERROR_SECTION_ALIGNMENT_INVALID",    "IMAGE_OPTIONAL_HEADER::SectionAlignment must be equal to FileAlignment for small alignments" },
		{"LDR_ERROR_SIZE_OF_IMAGE_TOO_BIG",        "IMAGE_OPTIONAL_HEADER::SizeOfImage is too big" },
		{"LDR_ERROR_INVALID_MACHINE32",            "IMAGE_FILE_HEADER::Machine is invalid for 32-bit optional header" },
		{"LDR_ERROR_INVALID_MACHINE64",            "IMAGE_FILE_HEADER::Machine is invalid for 64-bit optional header" },
		{"LDR_ERROR_SIZE_OF_HEADERS_INVALID",      "IMAGE_OPTIONAL_HEADER::SizeOfHeaders is greater than IMAGE_OPTIONAL_HEADER::SizeOfImage" },
		{"LDR_ERROR_SIZE_OF_OPTHDR_NOT_ALIGNED",   "IMAGE_OPTIONAL_HEADER::SizeOfHeaders is not aligned to 8 (64-bit Windows only)" },
		{"LDR_ERROR_SIZE_OF_IMAGE_ZERO",           "Number of PTEs for the entire image is zero" },
		{"LDR_ERROR_IMAGE_BASE_NOT_ALIGNED",       "IMAGE_OPTIONAL_HEADER::ImageBase is not aligned to 64KB" },
		{"LDR_ERROR_SIZE_OF_IMAGE_PTES_ZERO",      "Number of Page Table Entries for the image is zero" },
		{"LDR_ERROR_RAW_DATA_OVERFLOW",            "Overflow in section's raw data size" },
		{"LDR_ERROR_SECTION_HEADERS_OUT_OF_IMAGE", "Section headers are out of the image" },
		{"LDR_ERROR_SECTION_HEADERS_OVERFLOW",     "Image with single subsection: size of headers is near the end of range" },
		{"LDR_ERROR_SECTION_SIZE_MISMATCH",        "Image with single subsection: virtual values with rawdata values don't match" },
		{"LDR_ERROR_INVALID_SECTION_VA",           "Invalid virtual address of a section" },
		{"LDR_ERROR_INVALID_SECTION_VSIZE",        "Invalid virtual size of a section" },
		{"LDR_ERROR_INVALID_SECTION_RAWSIZE",      "Invalid raw data size of a section" },
		{"LDR_ERROR_INVALID_SIZE_OF_IMAGE",        "IMAGE_OPTIONAL_HEADER::SizeOfImage doesn't match the (header+sections)" },
		{"LDR_ERROR_FILE_IS_CUT",                  "The PE file is cut" },
		{"LDR_ERROR_FILE_IS_CUT_LOADABLE",         "The PE file is cut, but loadable" },
	};

	PELIB_IMAGE_FILE_MACHINE_ITERATOR::PELIB_IMAGE_FILE_MACHINE_ITERATOR()
	{

	}

	PELIB_IMAGE_FILE_MACHINE_ITERATOR::~PELIB_IMAGE_FILE_MACHINE_ITERATOR()
	{

	}

	bool PELIB_IMAGE_FILE_MACHINE_ITERATOR::isValidMachineCode(PELIB_IMAGE_FILE_MACHINE value) const
	{
		return find(all.begin(), all.end(), value) != all.end();
	}

	PELIB_IMAGE_FILE_MACHINE_ITERATOR::imageFileMachineIterator PELIB_IMAGE_FILE_MACHINE_ITERATOR::begin() const
	{
		return all.begin();
	}

	PELIB_IMAGE_FILE_MACHINE_ITERATOR::imageFileMachineIterator PELIB_IMAGE_FILE_MACHINE_ITERATOR::end() const
	{
		return all.end();
	}

	bool PELIB_IMAGE_SECTION_HEADER::biggerFileOffset(const PELIB_IMAGE_SECTION_HEADER& ish) const
	{
		return PointerToRawData < ish.PointerToRawData;
	}

	bool PELIB_IMAGE_SECTION_HEADER::biggerVirtualAddress(const PELIB_IMAGE_SECTION_HEADER& ish) const
	{
		return VirtualAddress < ish.VirtualAddress;
	}

	bool PELIB_IMAGE_SECTION_HEADER::isFullNameSet() const
	{
		return !StringTableName.empty();
	}

	unsigned int alignOffset(unsigned int uiOffset, unsigned int uiAlignment)
	{
		if (!uiAlignment) return uiAlignment;
		return (uiOffset % uiAlignment) ? uiOffset + (uiAlignment - uiOffset % uiAlignment) : uiOffset;
	}

	std::uint32_t AlignToSize(std::uint32_t ByteSize, std::uint32_t AlignSize)
	{
		return ((ByteSize + (AlignSize - 1)) & ~(AlignSize - 1));
	}

	std::uint32_t BytesToPages(std::uint32_t ByteSize)
	{
		return (ByteSize >> PELIB_PAGE_SIZE_SHIFT) + ((ByteSize & (PELIB_PAGE_SIZE - 1)) != 0);
	}

	std::uint64_t fileSize(const std::string& filename)
	{
		std::fstream file(filename.c_str());
		file.seekg(0, std::ios::end);
		return file.tellg();
	}

	std::uint64_t fileSize(std::ifstream& file)
	{
		std::streamoff oldpos = file.tellg();
		file.seekg(0, std::ios::end);
		std::streamoff filesize = file.tellg();
		file.seekg(oldpos, std::ios::beg);
		return filesize;
	}

	std::uint64_t fileSize(std::fstream& file)
	{
		std::streamoff oldpos = file.tellg();
		file.seekg(0, std::ios::end);
		std::streamoff filesize = file.tellg();
		file.seekg(oldpos, std::ios::beg);
		return filesize;
	}

	std::uint64_t fileSize(std::ofstream& file)
	{
		std::streamoff oldpos = file.tellp();
		file.seekp(0, std::ios::end);
		std::streamoff filesize = file.tellp();
		file.seekp(oldpos, std::ios::beg);
		return filesize;
	}

	const char * getLoaderErrorString(LoaderError ldrError, bool userFriendly)
	{
		std::size_t index = (std::size_t)ldrError;

		// When the index is within range
		if (index < LdrErrStrings.size())
		{
			return userFriendly ? LdrErrStrings[index].loaderErrorUserFriendly : LdrErrStrings[index].loaderErrorString;
		}

		// When the index is out of range
		return "LDR_ERROR_VALUE_OUT_OF_RANGE";
	}

	std::size_t getStringFromFileOffset(std::ifstream &ifFile, std::string &result, std::size_t fileOffset, std::size_t maxLength/* = 0*/)
	{
		result.clear();
		ifFile.clear();
		ifFile.seekg(fileOffset, std::ios::beg);
		if (!ifFile)
		{
			return 0;
		}

		char namebuffer[2] = { 0 };
		std::size_t size = 0;

		do
		{
			ifFile.read(namebuffer, 1);
			if (!ifFile || !namebuffer[0]) break;
			result += namebuffer;
			++size;
			if (maxLength && size == maxLength) break;
		} while (true);

		return size;
	}

	bool isEqualNc(const std::string& s1, const std::string& s2)
	{
		std::string t1 = s1;
		std::string t2 = s2;

		// No std:: to make VC++ happy
#ifdef _MSC_VER
		std::transform(t1.begin(), t1.end(), t1.begin(), [](unsigned char c) { return toupper(c); });
		std::transform(t2.begin(), t2.end(), t2.begin(), [](unsigned char c) { return toupper(c); });
#else
  // Weird syntax to make Borland C++ happy
		std::transform(t1.begin(), t1.end(), t1.begin(), (int(*)(int))std::toupper);
		std::transform(t2.begin(), t2.end(), t2.begin(), (int(*)(int))std::toupper);
#endif
		return t1 == t2;
	}

	PELIB_IMAGE_DOS_HEADER::PELIB_IMAGE_DOS_HEADER()
	{
		e_magic = 0;
		e_cblp = 0;
		e_cp = 0;
		e_crlc = 0;
		e_cparhdr = 0;
		e_minalloc = 0;
		e_maxalloc = 0;
		e_ss = 0;
		e_sp = 0;
		e_csum = 0;
		e_ip = 0;
		e_cs = 0;
		e_lfarlc = 0;
		e_ovno = 0;

		for (unsigned int i = 0; i < sizeof(e_res) / sizeof(e_res[0]); i++)
		{
			e_res[i] = 0;
		}

		e_oemid = 0;
		e_oeminfo = 0;

		for (unsigned int i = 0; i < sizeof(e_res2) / sizeof(e_res2[0]); i++)
		{
			e_res2[i] = 0;
		}

		e_lfanew = 0;
	}

	PELIB_EXP_FUNC_INFORMATION::PELIB_EXP_FUNC_INFORMATION()
	{
		addroffunc = 0;
		addrofname = 0;
		ordinal = 0;
	}

	PELIB_IMAGE_RESOURCE_DIRECTORY::PELIB_IMAGE_RESOURCE_DIRECTORY()
	{
		Characteristics = 0;
		TimeDateStamp = 0;
		MajorVersion = 0;
		MinorVersion = 0;
		NumberOfNamedEntries = 0;
		NumberOfIdEntries = 0;
	}

	PELIB_IMAGE_RESOURCE_DIRECTORY_ENTRY::PELIB_IMAGE_RESOURCE_DIRECTORY_ENTRY()
	{
		Name = 0;
		OffsetToData = 0;
	}

	bool PELIB_IMG_RES_DIR_ENTRY::operator<(const PELIB_IMG_RES_DIR_ENTRY& first) const
	{
		if ((irde.Name & PELIB_IMAGE_RESOURCE_NAME_IS_STRING) && (first.irde.Name & PELIB_IMAGE_RESOURCE_NAME_IS_STRING))
		{
			return wstrName < first.wstrName;
		}
		else if (irde.Name & PELIB_IMAGE_RESOURCE_NAME_IS_STRING)
		{
			return true;
		}
		else if (first.irde.Name & PELIB_IMAGE_RESOURCE_NAME_IS_STRING)
		{
			return false;
		}
		else
		{
			return irde.Name < first.irde.Name;
		}
	}

	PELIB_IMAGE_BASE_RELOCATION::PELIB_IMAGE_BASE_RELOCATION()
	{
		VirtualAddress = 0;
		SizeOfBlock = 0;
	}

	PELIB_IMAGE_COR20_HEADER::PELIB_IMAGE_COR20_HEADER()
	{
		cb = 0;
		MajorRuntimeVersion = 0;
		MinorRuntimeVersion = 0;
		MetaData.VirtualAddress = 0;
		MetaData.Size = 0;
		Flags = 0;
		EntryPointToken = 0;
		Resources.VirtualAddress = 0;
		Resources.Size = 0;
		StrongNameSignature.VirtualAddress = 0;
		StrongNameSignature.Size = 0;
		CodeManagerTable.VirtualAddress = 0;
		CodeManagerTable.Size = 0;
		VTableFixups.VirtualAddress = 0;
		VTableFixups.Size = 0;
		ExportAddressTableJumps.VirtualAddress = 0;
		ExportAddressTableJumps.Size = 0;
		ManagedNativeHeader.VirtualAddress = 0;
		ManagedNativeHeader.Size = 0;
	}

	PELIB_IMAGE_RESOURCE_DATA_ENTRY::PELIB_IMAGE_RESOURCE_DATA_ENTRY()
	{
		OffsetToData = 0;
		Size = 0;
		CodePage = 0;
		Reserved = 0;
	}

	PELIB_IMAGE_DEBUG_DIRECTORY::PELIB_IMAGE_DEBUG_DIRECTORY()
	{
		Characteristics = 0;
		TimeDateStamp = 0;
		MajorVersion = 0;
		MinorVersion = 0;
		Type = 0;
		SizeOfData = 0;
		AddressOfRawData = 0;
		PointerToRawData = 0;
	}

	/** Compares the passed filename to the struct's filename.
	* @param strModuleName A filename.
	* @return True, if the passed filename equals the struct's filename. The comparison is case-sensitive.
	**/
	bool PELIB_IMAGE_BOUND_DIRECTORY::equal(const std::string strModuleName2) const
	{
		return this->strModuleName == strModuleName2;
	}

	bool PELIB_EXP_FUNC_INFORMATION::equal(const std::string strFunctionName) const
	{
		return isEqualNc(this->funcname, strFunctionName);
	}

	/**
	* @param strFilename Name of a file.
	* @return Either PEFILE32, PEFILE64 or PEFILE_UNKNOWN
	**/
	unsigned int getFileType(const std::string strFilename)
	{
		PeFile32 pef(strFilename);

		// Attempt to read the DOS file header.
		if (pef.readMzHeader() != ERROR_NONE)
		{
			return PEFILE_UNKNOWN;
		}

		// Verify the DOS header
		if (!pef.mzHeader().isValid())
		{
			return PEFILE_UNKNOWN;
		}

		// Read PE header. Note that at this point, we read the header as if it was 32-bit PE file
		if (pef.readPeHeader() != ERROR_NONE)
		{
			return PEFILE_UNKNOWN;
		}

		word machine = pef.peHeader().getMachine();
		word magic = pef.peHeader().getMagic();

		// jk2012-02-20: make the PEFILE32 be the default return value
		// std::cout << "machine: " << machine << std::endl << "magic: " << magic << std::endl;
		if ((machine == PELIB_IMAGE_FILE_MACHINE_AMD64 || machine == PELIB_IMAGE_FILE_MACHINE_IA64) && magic == PELIB_IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		{
			return PEFILE64;
		}
		else
		{
			return PEFILE32;
		}

		/*
			word machine, magic;

			if (pef.readMzHeader() != ERROR_NONE)
			{
			  return PEFILE_UNKNOWN;
			}
			else if (!pef.mzHeader().isValid())
			{
				return PEFILE_UNKNOWN;
			}
			else if (pef.readPeHeader() != ERROR_NONE)
			{
			  return PEFILE_UNKNOWN;
			}

			machine = pef.peHeader().getMachine();
			magic = pef.peHeader().getMagic();

			// jk2012-02-20: make the PEFILE32 be the default return value
			// std::cout << "machine: " << machine << std::endl << "magic: " << magic << std::endl;
			if ((machine == PELIB_IMAGE_FILE_MACHINE_AMD64 || machine == PELIB_IMAGE_FILE_MACHINE_IA64) && magic == PELIB_IMAGE_NT_OPTIONAL_HDR64_MAGIC)
			{
			  return PEFILE64;
			}
			else
			{
			  return PEFILE32;
			}
		*/
		/*
			// jk 2012-02-20: original code
			if (machine == PELIB_IMAGE_FILE_MACHINE_I386 && magic == PELIB_IMAGE_NT_OPTIONAL_HDR32_MAGIC)
			{
			  return PEFILE32;
			}
			else if ((machine == PELIB_IMAGE_FILE_MACHINE_AMD64 || machine == PELIB_IMAGE_FILE_MACHINE_IA64) && magic == PELIB_IMAGE_NT_OPTIONAL_HDR64_MAGIC)
			{
			  return PEFILE64;
			}
			else
			{
			  return PEFILE_UNKNOWN;
			}
		*/
	}

	/**
	* Opens a PE file. The return type is either PeFile32 or PeFile64 object. If an error occurs the return
	* value is 0.
	* @param strFilename Name of a file.
	* @return Either a PeFile32 object, a PeFil64 object or 0.
	**/
	PeFile* openPeFile(const std::string& strFilename)
	{
		unsigned int type = getFileType(strFilename);

		if (type == PEFILE32)
		{
			return new PeFile32(strFilename);
		}
		else if (type == PEFILE64)
		{
			return new PeFile64(strFilename);
		}
		else
		{
			return 0;
		}
	}

	unsigned int PELIB_IMAGE_BOUND_DIRECTORY::size() const
	{
		unsigned int size = 0;
		for (unsigned int i = 0; i < moduleForwarders.size(); ++i)
		{
			size += moduleForwarders[i].size();
		}

		return size + PELIB_IMAGE_BOUND_IMPORT_DESCRIPTOR::size() + strModuleName.size() + 1;
	}
}
