#pragma once
#include <string>

namespace task
{
	///////////////////////////////////////////////////////////////////////////
	//
	//	������ ��������� ������, ���������� ����������� ���������.
	//
	///////////////////////////////////////////////////////////////////////////
	class CmdLineParser
	{
	public:
		CmdLineParser(int argc, char** argv);

		// --------------------------------------------------------------------
		// ���������� ����� ��������, ������� ��������� ���������.
		// --------------------------------------------------------------------
		inline std::string GetUrl() const;

		// --------------------------------------------------------------------
		// ���������� ����������, � ������� ��������� ��������� ��������.
		// --------------------------------------------------------------------
		inline std::string GetDirectory() const;

	private:
		std::string m_url;
		std::string m_directory;
	};

	///////////////////////////////////////////////////////////////////////////
}

// ----------------------------------------------------------------------------
inline std::string task::CmdLineParser::GetUrl() const
{
	return m_url;
}

// ----------------------------------------------------------------------------
inline std::string task::CmdLineParser::GetDirectory() const
{
	return m_directory;
}

// ----------------------------------------------------------------------------
