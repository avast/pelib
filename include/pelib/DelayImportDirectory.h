/**
 * @file DelayImportDirectory.h
 * @brief Class for delay import directory.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#ifndef DELAY_IMPORT_DIRECTORY_H
#define DELAY_IMPORT_DIRECTORY_H

#include "pelib/PeLibInc.h"
#include "pelib/PeHeader.h"

namespace PeLib
{
	/**
	 * This class handless delay import directory.
	 */
	template<int bits>
	class DelayImportDirectory
	{
		typedef typename std::vector<PELIB_IMAGE_DELAY_IMPORT_DIRECTORY_RECORD<bits> >::const_iterator DelayImportDirectoryIterator;
		typedef typename FieldSizes<bits>::VAR4_8 VAR4_8;

		private:
			std::vector<PELIB_IMAGE_DELAY_IMPORT_DIRECTORY_RECORD<bits> > records;

			void init()
			{
				records.clear();
			}

		public:
			DelayImportDirectory()
			{
				init();
			}

			~DelayImportDirectory()
			{

			}

			// Delay-import descriptors made by MS Visual C++ 6.0 has an old format
			// of delay import directory, where all entries are VAs (as opposite to RVAs from newer MS compilers).
			// We convert the delay-import directory entries to RVAs by checking 
			// whether their value is closer to DelayImportDescriptorVA or DelayImportDescriptorRVA
			VAR4_8 convertVAtoRVA(const PeHeaderT<bits>& peHeader, VAR4_8 valueToConvert)
			{
				// Ignore zero items
				if (valueToConvert != 0)
				{
					VAR4_8 delayImportRVA = peHeader.getIddDelayImportRva();
					VAR4_8 delayImportVA = peHeader.rvaToVa(delayImportRVA);

					if (std::abs(static_cast<int>(delayImportVA - valueToConvert)) <
						std::abs(static_cast<int>(delayImportRVA - valueToConvert)))
					{
						valueToConvert = valueToConvert - peHeader.getImageBase();
					}
				}

				return valueToConvert;
			}

			int read(const std::string& strFilename, const PeHeaderT<bits>& peHeader)
			{
				init();
				std::ifstream ifFile(strFilename, std::ios::binary);
				if (!ifFile)
				{
					return ERROR_OPENING_FILE;
				}

				std::uint64_t ulFileSize = fileSize(ifFile);
				std::uint64_t uiOffset = peHeader.rvaToOffset(peHeader.getIddDelayImportRva());
				if (uiOffset >= ulFileSize)
				{
					return ERROR_INVALID_FILE;
				}

				PELIB_IMAGE_DELAY_IMPORT_DIRECTORY_RECORD<bits> rec;
				std::vector<unsigned char> dump;
				dump.resize(PELIB_IMAGE_SIZEOF_DELAY_IMPORT_DIRECTORY_RECORD);

				// Keep loading until we encounter an entry filles with zeros
				for(std::size_t i = 0;; i += PELIB_IMAGE_SIZEOF_DELAY_IMPORT_DIRECTORY_RECORD)
				{
					InputBuffer inputbuffer(dump);

					// Read the n-th import sdirectory entry
					if (!ifFile.seekg(uiOffset + i, std::ios::beg))
						break;
					if (!ifFile.read(reinterpret_cast<char*>(dump.data()), PELIB_IMAGE_SIZEOF_DELAY_IMPORT_DIRECTORY_RECORD))
						break;

					rec.init();
					inputbuffer >> rec.Attributes;
					inputbuffer >> rec.NameRva;
					inputbuffer >> rec.ModuleHandleRva;
					inputbuffer >> rec.DelayImportAddressTableRva;
					inputbuffer >> rec.DelayImportNameTableRva;
					inputbuffer >> rec.BoundDelayImportTableRva;
					inputbuffer >> rec.UnloadDelayImportTableRva;
					inputbuffer >> rec.TimeStamp;
					if ( rec.Attributes == 0 && rec.NameRva == 0 && rec.ModuleHandleRva == 0 && rec.DelayImportAddressTableRva == 0 &&
						rec.DelayImportNameTableRva == 0 && rec.BoundDelayImportTableRva == 0 && rec.UnloadDelayImportTableRva == 0 &&
						rec.TimeStamp == 0)
					{
						break;
					}

					// Convert older (MS Visual C++ 6.0) delay-import descriptor to newer one.
					// Sample: 2775d97f8bdb3311ace960a42eee35dbec84b9d71a6abbacb26c14e83f5897e4
					rec.NameRva                    = (dword)convertVAtoRVA(peHeader, rec.NameRva);
					rec.ModuleHandleRva            = (dword)convertVAtoRVA(peHeader, rec.ModuleHandleRva);
					rec.DelayImportAddressTableRva = (dword)convertVAtoRVA(peHeader, rec.DelayImportAddressTableRva);
					rec.DelayImportNameTableRva    = (dword)convertVAtoRVA(peHeader, rec.DelayImportNameTableRva);
					rec.BoundDelayImportTableRva   = (dword)convertVAtoRVA(peHeader, rec.BoundDelayImportTableRva);
					rec.UnloadDelayImportTableRva  = (dword)convertVAtoRVA(peHeader, rec.UnloadDelayImportTableRva);

					rec.DelayImportAddressTableOffset = peHeader.rvaToOffset(rec.DelayImportAddressTableRva);
					rec.DelayImportNameTableOffset = peHeader.rvaToOffset(rec.DelayImportNameTableRva);

					// Get name of library
					getStringFromFileOffset(ifFile, rec.Name, (std::size_t)peHeader.rvaToOffset(rec.NameRva), IMPORT_LIBRARY_MAX_LENGTH);

					// Get names first
					// Address table is not guaranteed to be null-terminated and therefore we need to first read name table.
					ifFile.seekg(rec.DelayImportNameTableOffset, std::ios::beg);
					if(!ifFile)
					{
						return ERROR_INVALID_FILE;
					}

					std::vector<PELIB_VAR_SIZE<bits>> nameAddresses;
					do
					{
						PELIB_VAR_SIZE<bits> nameAddr;
						std::vector<byte> vBuffer(sizeof(nameAddr.Value));
						ifFile.read(reinterpret_cast<char*>(vBuffer.data()), sizeof(nameAddr.Value));
						if (!ifFile || ifFile.gcount() < sizeof(nameAddr.Value))
						{
							break;
						}

						InputBuffer inb(vBuffer);
						inb >> nameAddr.Value;

						// Value of zero means that this is the end of the bound import name table
						if (nameAddr.Value == 0)
							break;

						// If the highest bit is set, then it means that its not a name, but an ordinal
						// If not, we need to check whether this is an VA-based delay-import descriptor
						if (!(nameAddr.Value & PELIB_IMAGE_ORDINAL_FLAGS<bits>::PELIB_IMAGE_ORDINAL_FLAG))
						{
							nameAddr.Value = convertVAtoRVA(peHeader, nameAddr.Value);
						}

						nameAddresses.push_back(nameAddr);
					} while (true);

					// Get addresses
					ifFile.seekg(rec.DelayImportAddressTableOffset, std::ios::beg);
					if (!ifFile)
					{
						return ERROR_INVALID_FILE;
					}

					for (std::size_t i = 0, e = nameAddresses.size(); i < e; ++i)
					{
						PELIB_VAR_SIZE<bits> addr;
						std::vector<byte> vBuffer(sizeof(addr.Value));
						ifFile.read(reinterpret_cast<char*>(vBuffer.data()), sizeof(addr.Value));
						if (!ifFile || ifFile.gcount() < sizeof(addr.Value))
						{
							break;
						}

						InputBuffer inb(vBuffer);
						inb >> addr.Value;

						// The value of zero means terminator of the function table
						if (addr.Value == 0)
						{
							break;
						}

						// The table is always in the image itself
						if(peHeader.getImageBase() <= addr.Value && addr.Value < peHeader.getImageBase() + peHeader.getSizeOfImage())
						{
							addr.Value -= peHeader.getImageBase();
						}

						PELIB_DELAY_IMPORT<bits> function;
						function.address.Value = addr.Value;
						rec.addFunction(function);
					}

					for (std::size_t i = 0, e = rec.getNumberOfFunctions(); i < e; ++i)
					{
						auto *actFunc = rec.getFunction(i);
						if (!actFunc)
						{
							continue;
						}

						// Delay Import By Name?
						if ((nameAddresses[i].Value & PELIB_IMAGE_ORDINAL_FLAGS<bits>::PELIB_IMAGE_ORDINAL_FLAG) == 0)
						{
							std::vector<byte> vBuffer(sizeof(actFunc->hint));
							ifFile.seekg(peHeader.rvaToOffset(nameAddresses[i].Value), std::ios::beg);
							ifFile.read(reinterpret_cast<char*>(vBuffer.data()), sizeof(actFunc->hint));
							if (!ifFile || ifFile.gcount() < sizeof(actFunc->hint))
							{
								break;
							}

							InputBuffer inb(vBuffer);
							inb >> actFunc->hint;
							getStringFromFileOffset(ifFile, actFunc->fname, ifFile.tellg(), IMPORT_SYMBOL_MAX_LENGTH);
						}
						else
						{
							actFunc->Ordinal = nameAddresses[i].Value & 0xFFFF;
							actFunc->hint = 0;
						}
					}

					records.push_back(rec);
				}

				return ERROR_NONE;
			}

			std::size_t getNumberOfFiles() const
			{
				return records.size();
			}

			const PELIB_IMAGE_DELAY_IMPORT_DIRECTORY_RECORD<bits> *getFile(std::size_t index) const
			{
				return index < getNumberOfFiles() ? &records[index] : nullptr;
			}

			DelayImportDirectoryIterator begin() const
			{
				return records.begin();
			}

			DelayImportDirectoryIterator end() const
			{
				return records.end();
			}
	};
}

#endif
