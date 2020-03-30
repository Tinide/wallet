#pragma once
#include "Markup.h"
#include "log.h"

class CReadConfig
{
public:
	static std::string ReadXMLConfig(std::string xmlPath,std::string field_0,std::string field_1,std::string field_2 = "")
	{
		CMarkup xml;
		if( xml.Load(xmlPath) == false )
		{
			LOG_ERROR("{}����ʧ��1,���������ļ��Ƿ���� �� ��ʽ�Ƿ���ȷ!{}", xmlPath,xml.GetError().c_str());
			return "";
		}
		
		if( xml.FindElem(field_0) == false )
		{
			LOG_ERROR("{}----{} ������,���������ļ�!", xmlPath,field_0);
			return "";
		}
		xml.IntoElem();

		if( xml.FindElem(field_1) == false )
		{
			LOG_ERROR("{}----{} ������,���������ļ�!", xmlPath,field_1.c_str());
			return "";
		}
		if( "" == field_2 )
		{
			return xml.GetData();
		}
		
		if( xml.FindChildElem(field_2) == false )
		{
			LOG_ERROR("{}----{} ������,���������ļ�!", xmlPath,field_2.c_str());
			return "";
		}
		
		return xml.GetChildData();
	}

	static std::vector<std::string> ReadXMLVec(std::string xmlPath,std::vector<std::string> path)
	{
		std::vector<std::string> vec;

		CMarkup xml;
		if (xml.Load(xmlPath) == false)
		{
			LOG_ERROR("{}����ʧ��,���������ļ��Ƿ���� �� ��ʽ�Ƿ���ȷ!{}", xmlPath, xml.GetError().c_str());
			return vec;
		}

		for (int i = 0; i < int(path.size()); i++)
		{
			if (i != path.size() - 1)
			{
				if (xml.FindElem(path[i]))
				{
					xml.IntoElem();
				}
				else
				{
					LOG_ERROR("{},{}�ڵ㲻����", xmlPath, path[i]);
					break;
				}
			}
			else
			{
				while (xml.FindElem(path[i]))
				{
					vec.push_back(xml.GetData());
				}
			}
		}
		return vec;
	}
};



class CWriteConfig
{
public:
	static bool WriteXMLConfig(std::string value,std::string field_1,std::string field_2 = "")
	{
		std::string XmlPath = SZAPPNAME;
		XmlPath += ".xml";

		CMarkup xml;
		if( xml.Load(XmlPath) == false )
		{
			LOG_ERROR("{}.xml����ʧ��2,���������ļ��Ƿ���� �� ��ʽ�Ƿ���ȷ!{}",SZAPPNAME,xml.GetError().c_str());
			return false;
		}
		if( xml.FindElem("CONFIG") == false )
		{
			LOG_ERROR("{}.xml----CONFIG ������,���������ļ�!", SZAPPNAME);
			return false;
		}
		xml.IntoElem();

		do 
		{
			if( xml.FindElem(field_1) == false )
			{
				LOG_ERROR("{}.xml----{} ������,���������ļ�!", SZAPPNAME,field_1.c_str());
				return false;
			}
			if( "" == field_2 )
			{
				xml.SetData(value);
				break;
			}

			if( xml.FindChildElem(field_2) == false )
			{
				LOG_ERROR("{}.xml----{} ������,���������ļ�!", SZAPPNAME,field_2.c_str());
				return false;
			}
			xml.SetChildData(value);
		} while (0);
		return xml.Save(XmlPath);
	}


	static bool WriteXMLConfig(int value,std::string field_1,std::string field_2 = "")
	{
		std::string XmlPath = SZAPPNAME;
		XmlPath += ".xml";

		CMarkup xml;
		if( xml.Load(XmlPath) == false )
		{
			LOG_ERROR("{}.xml����ʧ��3,���������ļ��Ƿ���� �� ��ʽ�Ƿ���ȷ!{}",SZAPPNAME,xml.GetError().c_str());
			return false;
		}
		if( xml.FindElem("CONFIG") == false )
		{
			LOG_ERROR("{}.xml----CONFIG ������,���������ļ�!", SZAPPNAME);
			return false;
		}
		xml.IntoElem();

		do 
		{
			if( xml.FindElem(field_1) == false )
			{
				LOG_ERROR("{}.xml----{} ������,���������ļ�!", SZAPPNAME,field_1.c_str());
				return false;
			}
			if( "" == field_2 )
			{
				xml.SetData(value);
				break;
			}

			if( xml.FindChildElem(field_2) == false )
			{
				LOG_ERROR("{}.xml----{} ������,���������ļ�!", SZAPPNAME,field_2.c_str());
				return false;
			}
			xml.SetChildData(value);
		} while (0);
		return xml.Save(XmlPath);
	}
};

#define  READ_CFG CReadConfig::ReadXMLConfig