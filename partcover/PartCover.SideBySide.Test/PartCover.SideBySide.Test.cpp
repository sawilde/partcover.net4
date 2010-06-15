// PartCover.SideBySide.Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#import "../bin/PartCover.TestCom.Net2.tlb" no_namespace
#import "../bin/PartCover.TestCom.Net4.tlb" no_namespace

int _tmain(int argc, _TCHAR* argv[])
{
	::CoInitialize(NULL);

	IDispInterface2Ptr ptr2;
	IDispInterface4Ptr ptr4;

	ptr4.CreateInstance("PartCover.TestCom.Net4.Class_NET4_COM");
	ptr2.CreateInstance("PartCover.TestCom.Net2.Class_NET2_COM");

	ptr2->WhatEnvironment();

	ptr4->WhatEnvironment();

	ptr4 = 0;
	ptr2 = 0;

	::CoUninitialize();

	return 0;
}

