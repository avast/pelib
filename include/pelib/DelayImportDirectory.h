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

					unsigned int uiDelayImportsRva = peHeader.offsetToRva(uiOffset);
					unsigned int uiDelayImportsVa = (unsigned int)peHeader.rvaToVa(uiDelayImportsRva);

					// Convert absolute virtual addresses to relative virtual addresses.
					// We do this by calculating whether address is closer to RVA of directory or VA of the directory.
					if (std::abs(static_cast<int>(uiDelayImportsVa - rec.NameRva)) <
						std::abs(static_cast<int>(uiDelayImportsRva - rec.NameRva)))
					{
						rec.NameRva -= peHeader.getImageBase();
					}
					if (std::abs(static_cast<int>(uiDelayImportsVa - rec.ModuleHandleRva)) <
						std::abs(static_cast<int>(uiDelayImportsRva - rec.ModuleHandleRva)))
					{
						rec.ModuleHandleRva -= peHeader.getImageBase();
					}
					if (std::abs(static_cast<int>(uiDelayImportsVa - rec.DelayImportAddressTableRva)) <
						std::abs(static_cast<int>(uiDelayImportsRva - rec.DelayImportAddressTableRva)))
					{
						rec.DelayImportAddressTableRva -= peHeader.getImageBase();
					}
					if (std::abs(static_cast<int>(uiDelayImportsVa - rec.DelayImportNameTableRva)) <
						std::abs(static_cast<int>(uiDelayImportsRva - rec.DelayImportNameTableRva)))
					{
						rec.DelayImportNameTableRva -= peHeader.getImageBase();
					}
					if (std::abs(static_cast<int>(uiDelayImportsVa - rec.BoundDelayImportTableRva)) <
						std::abs(static_cast<int>(uiDelayImportsRva - rec.BoundDelayImportTableRva)))
					{
						rec.BoundDelayImportTableRva -= peHeader.getImageBase();
					}
					if (std::abs(static_cast<int>(uiDelayImportsVa - rec.UnloadDelayImportTableRva)) <
						std::abs(static_cast<int>(uiDelayImportsRva - rec.UnloadDelayImportTableRva)))
					{
						rec.UnloadDelayImportTableRva -= peHeader.getImageBase();
					}
					rec.DelayImportAddressTableOffset = peHeader.rvaToOffset(rec.DelayImportAddressTableRva);
					rec.DelayImportNameTableOffset = peHeader.rvaToOffset(rec.DelayImportNameTableRva);

					// Get name of library
					getStringFromFileOffset(ifFile, rec.Name, peHeader.rvaToOffset(rec.NameRva), IMPORT_LIBRARY_MAX_LENGTH);

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

						if (nameAddr.Value == 0)
						{
							break;
						}
						else if (std::abs(static_cast<std::int64_t>(nameAddr.Value) - uiDelayImportsVa) <
							std::abs(static_cast<std::int64_t>(nameAddr.Value) - uiDelayImportsRva))
						{
							nameAddr.Value -= peHeader.getImageBase();
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

						if (addr.Value == 0)
						{
							break;
						}
						else if (std::abs(static_cast<std::int64_t>(addr.Value) - uiDelayImportsVa) <
							std::abs(static_cast<std::int64_t>(addr.Value) - uiDelayImportsRva))
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
