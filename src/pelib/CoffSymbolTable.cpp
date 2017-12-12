/**
 * @file CoffSymbolTable.cpp
 * @brief Class for COFF symbol table.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#include <algorithm>

#include "pelib/PeLibInc.h"
#include "pelib/CoffSymbolTable.h"

namespace PeLib
{
	CoffSymbolTable::CoffSymbolTable() : stringTableSize(0), numberOfStoredSymbols(0)
	{

	}

	CoffSymbolTable::~CoffSymbolTable()
	{

	}

	void CoffSymbolTable::read(InputBuffer& inputbuffer, unsigned int uiSize)
	{
		PELIB_IMAGE_COFF_SYMBOL symbol;

		for (std::size_t i = 0, e = uiSize / PELIB_IMAGE_SIZEOF_COFF_SYMBOL; i < e; ++i)
		{
			symbol.Name.clear();
			dword Zeroes, NameOffset;
			inputbuffer >> Zeroes;
			inputbuffer >> NameOffset;
			inputbuffer >> symbol.Value;
			inputbuffer >> symbol.SectionNumber;
			inputbuffer >> symbol.TypeComplex;
			inputbuffer >> symbol.TypeSimple;
			inputbuffer >> symbol.StorageClass;
			inputbuffer >> symbol.NumberOfAuxSymbols;
			symbol.Index = i;
			if (!Zeroes)
			{
				if (stringTableSize && NameOffset)
				{
					for (std::size_t j = NameOffset; j < stringTableSize && stringTable[j] != '\0'; ++j)
					{
						// If we have symbol name with length of 96 and it contains non-printable character, stop there because it does not seem to be valid.
						if (j - NameOffset == COFF_SYMBOL_NAME_MAX_LENGTH)
						{
							auto nonPrintableChars = std::count_if(symbol.Name.begin(), symbol.Name.end(), [](unsigned char c) { return !isprint(c); });
							if (nonPrintableChars != 0)
								break;
						}

						symbol.Name += stringTable[j];
					}
				}
			}
			else
			{
				for (std::size_t j = i * PELIB_IMAGE_SIZEOF_COFF_SYMBOL, k = 0; k < 8 && symbolTableDump[j] != '\0'; ++j, ++k)
				{
					symbol.Name += symbolTableDump[j];
				}
			}

			i += symbol.NumberOfAuxSymbols;
			inputbuffer.move(symbol.NumberOfAuxSymbols * PELIB_IMAGE_SIZEOF_COFF_SYMBOL);
			symbolTable.push_back(symbol);
		}

		numberOfStoredSymbols = symbolTable.size();
	}

	int CoffSymbolTable::read(const std::string& strFilename, unsigned int uiOffset, unsigned int uiSize)
	{
		std::ifstream ifFile(strFilename, std::ios::binary);
		if (!ifFile)
		{
			return ERROR_OPENING_FILE;
		}

		const std::size_t ulFileSize = fileSize(ifFile);
		const std::size_t stringTableOffset = uiOffset + uiSize;
		if (stringTableOffset >= ulFileSize || uiOffset >= ulFileSize)
		{
			return ERROR_INVALID_FILE;
		}

		ifFile.seekg(uiOffset, std::ios::beg);
		symbolTableDump.resize(uiSize);
		ifFile.read(reinterpret_cast<char*>(symbolTableDump.data()), uiSize);
		InputBuffer ibBuffer(symbolTableDump);

		// read size of string table
		if (ulFileSize >= stringTableOffset + 4)
		{
			stringTable.resize(4);
			ifFile.read(reinterpret_cast<char*>(stringTable.data()), 4);
			InputBuffer strBuf(stringTable);
			strBuf >> stringTableSize;
		}

		if (ifFile.gcount() < 4)
		{
			stringTableSize = ifFile.gcount();
		}
		else if (ifFile.gcount() == 4 && stringTableSize < 4)
		{
			stringTableSize = 4;
		}

		if (stringTableSize > ulFileSize || uiOffset + stringTableSize > ulFileSize)
		{
			stringTableSize = ulFileSize - uiOffset;
		}

		// read string table
		if (stringTableSize > 4)
		{
			stringTable.resize(stringTableSize);
			ifFile.read(reinterpret_cast<char*>(stringTable.data() + 4), stringTableSize - 4);
		}

		read(ibBuffer, uiSize);

		return ERROR_NONE;
	}

	std::size_t CoffSymbolTable::getSizeOfStringTable() const
	{
		return stringTableSize;
	}

	std::size_t CoffSymbolTable::getNumberOfStoredSymbols() const
	{
		return numberOfStoredSymbols;
	}

	dword CoffSymbolTable::getSymbolIndex(std::size_t ulSymbol) const
	{
		return symbolTable[ulSymbol].Index;
	}

	std::string CoffSymbolTable::getSymbolName(std::size_t ulSymbol) const
	{
		return symbolTable[ulSymbol].Name;
	}

	dword CoffSymbolTable::getSymbolValue(std::size_t ulSymbol) const
	{
		return symbolTable[ulSymbol].Value;
	}

	word CoffSymbolTable::getSymbolSectionNumber(std::size_t ulSymbol) const
	{
		return symbolTable[ulSymbol].SectionNumber;
	}

	byte CoffSymbolTable::getSymbolTypeComplex(std::size_t ulSymbol) const
	{
		return symbolTable[ulSymbol].TypeComplex;
	}

	byte CoffSymbolTable::getSymbolTypeSimple(std::size_t ulSymbol) const
	{
		return symbolTable[ulSymbol].TypeSimple;
	}

	byte CoffSymbolTable::getSymbolStorageClass(std::size_t ulSymbol) const
	{
		return symbolTable[ulSymbol].StorageClass;
	}

	byte CoffSymbolTable::getSymbolNumberOfAuxSymbols(std::size_t ulSymbol) const
	{
		return symbolTable[ulSymbol].NumberOfAuxSymbols;
	}
}
