#include "ExeFile.h"
#include <algorithm>
#include <fstream>


bool hl::ExeFile::loadFromFile(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	size_t size = (size_t)file.tellg();
	file.seekg(0, std::ios::beg);
	if (size == (size_t)-1)
	{
		return false;
	}

	std::vector<char> buf(size);
	if (file.read(buf.data(), size))
	{
		return loadFromMem((uintptr_t)buf.data());
	}
	return false;
}

bool hl::ExeFile::hasRelocs() const
{
	if (!m_valid)
	{
		throw std::runtime_error("no exe file loaded");
	}

	return !m_relocs.empty();
}

bool hl::ExeFile::isReloc(uintptr_t rva) const
{
	if (!m_valid)
	{
		throw std::runtime_error("no exe file loaded");
	}

	auto it = std::find(m_relocs.begin(), m_relocs.end(), rva);
	return it != m_relocs.end();
}

uintptr_t hl::ExeFile::getExport(const std::string& name) const
{
	auto it = m_exports.find(name);
	if (it != m_exports.end())
	{
		return it->second;
	}

	return 0;
}
#include <Windows.h>


class hl::ExeFileImpl
{
public:
	IMAGE_DOS_HEADER* dosHeader = nullptr;
	IMAGE_NT_HEADERS* peHeader = nullptr;
	std::vector<IMAGE_SECTION_HEADER*> sectionHeaders;
};


static std::vector<uintptr_t> LoadRelocs(uintptr_t relocData)
{
	std::vector<uintptr_t> relocs;

	DWORD* regionPtr = (DWORD*)relocData;
	WORD* relocPtr = (WORD*)relocData;

	while (true)
	{
		DWORD rvaRegion = *regionPtr;
		DWORD regionSize = *(regionPtr + 1);

		if (!regionSize)
		{
			break;
		}

		regionPtr += regionSize / 4;
		relocPtr += 4;

		while (relocPtr < (WORD*)regionPtr)
		{
			WORD relocLow = *relocPtr++;
			int type = relocLow >> 12;
			relocLow &= 0x0fff;

			if (type == IMAGE_REL_BASED_HIGHLOW)
			{
				uintptr_t reloc = rvaRegion | relocLow;
				relocs.push_back(reloc);
			}
		}
	}

	return relocs;
}


hl::ExeFile::ExeFile()
{
	m_impl = std::make_unique<ExeFileImpl>();
}

hl::ExeFile::~ExeFile() {}


bool hl::ExeFile::loadFromMem(uintptr_t moduleBase)
{
	m_valid = false;

	// Load DOS header and verify "MZ" signature.
	m_impl->dosHeader = (IMAGE_DOS_HEADER*)moduleBase;
	if (m_impl->dosHeader->e_magic != IMAGE_DOS_SIGNATURE || m_impl->dosHeader->e_lfanew > 0x200)
	{
		return false;
	}

	// Load PE header.
	m_impl->peHeader = (IMAGE_NT_HEADERS*)((uintptr_t)moduleBase + m_impl->dosHeader->e_lfanew);
	if (m_impl->peHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		return false;
	}

	// Only support 32-bit images for now.
	if (m_impl->peHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ||
		!(m_impl->peHeader->FileHeader.Characteristics & IMAGE_FILE_32BIT_MACHINE))
	{
		return false;
	}

	// Load section headers.
	int nSections = m_impl->peHeader->FileHeader.NumberOfSections;
	for (int i = 0; i < nSections; i++)
	{
		auto sectionHeader = (IMAGE_SECTION_HEADER*)((uintptr_t)m_impl->peHeader + sizeof(IMAGE_NT_HEADERS) +
			i * sizeof(IMAGE_SECTION_HEADER));
		m_impl->sectionHeaders.push_back(sectionHeader);
	}

	// Load relocs.
	auto relocDir = &m_impl->peHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	if (relocDir->VirtualAddress != 0)
	{
		m_relocs = LoadRelocs(moduleBase + relocDir->VirtualAddress);
	}

	m_valid = true;
	return true;
}