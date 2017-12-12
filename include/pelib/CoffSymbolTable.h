/**
 * @file CoffSymbolTable.h
 * @brief Class for COFF symbol table.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#ifndef COFFSYMBOLTABLE_H
#define COFFSYMBOLTABLE_H

#include <vector>

namespace PeLib
{
	/**
	 * This class handless the COFF symbol table.
	 */
	class CoffSymbolTable
	{
		private:
			dword stringTableSize;
			dword numberOfStoredSymbols;
			std::vector<unsigned char> stringTable;
			std::vector<unsigned char> symbolTableDump;
			std::vector<PELIB_IMAGE_COFF_SYMBOL> symbolTable;

			void read(InputBuffer& inputbuffer, unsigned int uiSize);
		public:
			CoffSymbolTable();
			~CoffSymbolTable();

			int read(const std::string& strFilename, unsigned int uiOffset, unsigned int uiSize);
			std::size_t getSizeOfStringTable() const;
			std::size_t getNumberOfStoredSymbols() const;
			dword getSymbolIndex(std::size_t ulSymbol) const;
			std::string getSymbolName(std::size_t ulSymbol) const;
			dword getSymbolValue(std::size_t ulSymbol) const;
			word getSymbolSectionNumber(std::size_t ulSymbol) const;
			byte getSymbolTypeComplex(std::size_t ulSymbol) const;
			byte getSymbolTypeSimple(std::size_t ulSymbol) const;
			byte getSymbolStorageClass(std::size_t ulSymbol) const;
			byte getSymbolNumberOfAuxSymbols(std::size_t ulSymbol) const;
	};
}

#endif
