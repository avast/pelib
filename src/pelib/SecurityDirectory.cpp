/**
 * @file SecurityDirectory.cpp
 * @brief Class for security directory.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#include "pelib/PeLibInc.h"
#include "pelib/SecurityDirectory.h"

namespace PeLib
{
	unsigned int SecurityDirectory::calcNumberOfCertificates() const
	{
		return m_certs.size();
	}

	const std::vector<unsigned char>& SecurityDirectory::getCertificate(std::size_t index) const
	{
		return m_certs[index].Certificate;
	}

	int SecurityDirectory::read(const std::string& strFilename, unsigned int uiOffset, unsigned int uiSize)
	{
		std::ifstream ifFile(strFilename.c_str(), std::ios::binary);

		if (!ifFile)
		{
			return ERROR_OPENING_FILE;
		}

		std::uint64_t ulFileSize = fileSize(ifFile);
		if (ulFileSize < uiOffset + uiSize)
		{
			return ERROR_INVALID_FILE;
		}

		ifFile.seekg(uiOffset, std::ios::beg);

		std::vector<unsigned char> vCertDirectory(uiSize);
		ifFile.read(reinterpret_cast<char*>(vCertDirectory.data()), uiSize);

		InputBuffer inpBuffer(vCertDirectory);

		unsigned bytesRead = 0;
		while (bytesRead < uiSize)
		{
			PELIB_IMAGE_CERTIFICATE_ENTRY cert;
			inpBuffer >> cert.Length;
			inpBuffer >> cert.Revision;
			inpBuffer >> cert.CertificateType;

			if ((cert.Length <= PELIB_IMAGE_CERTIFICATE_ENTRY::size() ||
				((cert.Revision != PELIB_WIN_CERT_REVISION_1_0) && (cert.Revision != PELIB_WIN_CERT_REVISION_2_0)) ||
				(cert.CertificateType != PELIB_WIN_CERT_TYPE_PKCS_SIGNED_DATA)))
			{
				return ERROR_INVALID_FILE;
			}

			cert.Certificate.resize(cert.Length - PELIB_IMAGE_CERTIFICATE_ENTRY::size());
			inpBuffer.read(reinterpret_cast<char*>(cert.Certificate.data()), cert.Certificate.size());

			bytesRead += cert.Length;
			m_certs.push_back(cert);
		}

		return ERROR_NONE;
	}
}
