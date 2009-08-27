#ifndef _mytext_h_
#define	_mytext_h_

#include <string.h>
#include <math.h>
#include <algorithm>
#include "SimpleIni.h"

//---------------------------------------------------------------------------
//��������������� �� cp1251 � ���8 � �������� � ���
// ������� ����� � �� ���� � utf
const char asc[] = "������������������������������������������������������������������";
const unsigned char utf[] = {0xD0,0x81,0xD1,0x91,0xD0,0x90,0xD0,0x91,0xD0,0x92,0xD0,0x93,0xD0,0x94,0xD0,0x95,0xD0,0x96,0xD0,0x97,0xD0,0x98,0xD0,0x99,0xD0,0x9A,0xD0,0x9B,0xD0,0x9C,0xD0,0x9D,0xD0,0x9E,0xD0,0x9F,0xD0,0xA0,0xD0,0xA1,0xD0,0xA2,0xD0,0xA3,0xD0,0xA4,0xD0,0xA5,0xD0,0xA6,0xD0,0xA7,0xD0,0xA8,0xD0,0xA9,0xD0,0xAA,0xD0,0xAB,0xD0,0xAC,0xD0,0xAD,0xD0,0xAE,0xD0,0xAF,0xD0,0xB0,0xD0,0xB1,0xD0,0xB2,0xD0,0xB3,0xD0,0xB4,0xD0,0xB5,0xD0,0xB6,0xD0,0xB7,0xD0,0xB8,0xD0,0xB9,0xD0,0xBA,0xD0,0xBB,0xD0,0xBC,0xD0,0xBD,0xD0,0xBE,0xD0,0xBF,0xD1,0x80,0xD1,0x81,0xD1,0x82,0xD1,0x83,0xD1,0x84,0xD1,0x85,0xD1,0x86,0xD1,0x87,0xD1,0x88,0xD1,0x89,0xD1,0x8A,0xD1,0x8B,0xD1,0x8C,0xD1,0x8D,0xD1,0x8E,0xD1,0x8F};

SI_Error load_cp1251_file(CSimpleIni *ini, FILE * a_fpFile) {
    // load the raw file data
    int retval = fseek(a_fpFile, 0, SEEK_END);
    if (retval != 0) {
        return SI_FILE;
    }
    long lSize = ftell(a_fpFile);
    if (lSize < 0) {
        return SI_FILE;
    }
    if (lSize == 0) {
        return SI_OK;
    }
    char * pData = new char[lSize];
    if (!pData) {
        return SI_NOMEM;
    }
    fseek(a_fpFile, 0, SEEK_SET);
    size_t uRead = fread(pData, sizeof(char), lSize, a_fpFile);
    if (uRead != (size_t) lSize) {
        delete[] pData;
        return SI_FILE;
    }
	
	char *out = new char [(strlen(pData)+1)*2];
	if (!out) {
        return SI_NOMEM;
    }
	char *in = pData;
	unsigned int rus  = 0;
	unsigned long len = 0;
	while (*in) {
		if ((unsigned char)*in < 0x80) {
			out[len++] = *in; // ������ � ����������� ������ (�����, �������� � �.�.)
		} else {
			rus = strchr(asc,*in)-(char*)asc;
			if (rus>sizeof(asc)) {
				out[len++] = '.'; // ������ �� ������, �������� ��� ������
			} else {
				rus<<=1;
				out[len++] = utf[rus]; // ������� ����� ���������� ����� �������
				out[len++] = utf[rus+1];
			}
		}
		in++;
	}
	out[len++] = 0;
    delete[] pData;
    SI_Error rc = (*ini).Load(out, len);
    delete[] out;
    return rc;
}
int aa = 0;
SI_Error load_cp1251_ini_file(CSimpleIni *ini, const char * a_pszFile) {
	FILE * fp = NULL;
    fp = fopen(a_pszFile, "rb");
    if (!fp) {
        return SI_FILE;
    }
    SI_Error rc = load_cp1251_file(ini, fp);
    fclose(fp);
    return rc;
}

//�������� ������ ����� �� ������ ���� 1,4,5 � ��������� ��������� �� ��������� (4,5)
int get_int_from_string(char ** str) {
	if (strlen(*str) == 0) {return -1;}
	int ret = 0;
	unsigned int x = strcspn(*str, ",");
	ret = atoi(*str);
	if (strlen(*str) == x) { 
		*str = *str + x;
	} else {
		*str = *str + x + 1;
	}
	return ret;
}

//�� ������ ���� ��1,��4,��5 ���������� ��������� �� ��������� ������� ��4,��5
//���������� ������ ��1(��2,��3),��4 ��������� ����� �� ��4
//���� � ������ ������ ���� ������ " �� ���� ������ ���� ����������� �� ������ ������
//str - ��������� �� ������ ������
//len - ��������� �� ���������� ���� ��������� ������ �������� �������
char * get_station_from_string(char ** str, int * len, bool br = false) { 
	while(*str[0] == ' ' || *str[0] == '-') {
		*str = *str + 1;
	}
	
	if (strlen(*str) == 0) {return NULL;}
	char *dst = *str;
	int curcomma = strcspn(dst, ",");
	int curio = strcspn(dst, "(");
	int curiz = strcspn(dst, ")");
	int curquote = strcspn(dst, "\"");
	//���� ������ ������ ", ���� �����������
	if (curquote == 0) {
		dst = dst + 1;
		*str = *str + 1;
		//���� ����������� ������
		*len = strcspn(dst, "\"");
		//���� ������ ������
		if (*len != (int)strlen(dst)) {
			//��������� ��������� �� ������ ����� �������� �������
			dst += (*len + 1);
			//���� ����� ������ ���������� NULL
			if (strlen(dst) == 0) {
				return NULL;
			}
			
			//������ ���� ���� ������� ���� ����������� ������ � ����������� ���-�� �� ���
			if (strcspn(dst, ",") == 0) {
				dst += 1;
			} else {
				if (br == false) {
					curio = strcspn(dst, "(");
					curiz = strcspn(dst, ")");
					if (curio == 0 && curiz != (int)strlen(dst)) {
						dst += curiz + 1;
						//���� ����� ������ ���������� NULL
						if (strlen(dst) == 0) {
							return NULL;
						}
						if (strcspn(dst, ",") == 0) {
							dst += 1;
						} else {
							*len = -1;
							return NULL;
						}
					} else {
						*len = -1;
						return NULL;
					}
				} else {
					if (strcspn(dst, ")") == 0) {
						dst += 1;
					} else {
						*len = -1;
						return NULL;
					}
				}
			}
		} else {
			*len = -1;
			return NULL;
		}
	} else {
		//���� ������� ����� �� ������
		if ((curcomma <= curio && br == false) || (curcomma <= curiz && br == true)) {
			*len = curcomma;
			//���� �� ����� ������ ��������� �� ������ ����� �������
			if (curcomma != (int)strlen(dst)) {
				dst += curcomma + 1;
			} else {
				return NULL;
			}
		} else {
			if (br == false) {
				*len = curio;
				//���� ���� ����������� ������ ��������� �� ������ ����� ���
				if (curiz != (int)strlen(dst)) {
					dst += curiz + 1;
					//���� ����� ������ ���������� NULL
					if (strlen(dst) == 0) {
						return NULL;
					}
					//���� �� ����� ������ �� ������ ������ �� ����� "," ������ ������
					if (strcspn(dst, ",") == 0) {
						dst += 1;
					} else {
						*len = -1;
						return NULL;
					}
				} else {
					*len = -1;
					return NULL;
				}
			} else {
				*len = curiz;
				//���� ���� ����������� ������ ��������� �� ������ ����� ���
				dst += curiz + 1;
				//���� ����� ������ ���������� NULL
				if (strlen(dst) == 0) {
					return NULL;
				}
				//���� �� ����� ������ �� ����� ������ �� ����� "," ������ ������
				if (strcspn(dst, ",") != 0) {
					*len = -1;
					return NULL;
				}
			}
		}
	}
	return dst;
}


#endif	// _mytext_h_
