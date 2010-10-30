/*
 * Copyright (C) 2010 Lars Hall
 *
 * This file is part of BranchCov.

 * BranchCov is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BranchCov is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with BranchCov.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

void __tracer(const char *filename, const char *scope,
    const char *function_name, int lineno)
{
    /*FILE *f = fopen("out.lis", "wt");
    fprintf(f, "\nscope:%s function_name:%s", scope, function_name);
    fclose(f);*/
    printf("\n##filename:%s scope:%s function_name:%s lineno:%i \n",
        filename, scope, function_name, lineno);
}

void testSwitch(int s)
{
    switch (s)
    {
        case 1:
            break;
        case 2:
        case 3:
        {
            printf("testswitch: s == 3\n");
            break;
        }
        default:
            break;
    }
}

// Test of nested namespaces and classes
namespace a
{
    namespace b
    {
        class c
        {
            public:
                class d
                {
                    public:
                        void test()
                        {
                            int x = 0;
                            if (x == 0)
                                printf("nested: x == 0\n");
                            else
                                printf("nested: x != 0\n");
                        }
                };
        };
    }
}

class Base
{
    public:
        void baseTest()
        {
            int x = 10;
            if (x == 10)
                printf("basetest: constructor\n");
        }
};

class Inherit : private Base
{
    public:
        void test()
        {
            baseTest();
        }
};

namespace ns
{
    class TestCtorDtor
    {
        public:
            class TestClass
            {
                public:
                    TestClass()
                    {
                        int x = 0;
                        if (x == 0)
                            printf("testclass: constructor\n");
                    }
                    ~TestClass()
                    {
                        int x = 0;
                        if (x == 0)
                            printf("testclass: destructor\n");
                    }
            };
    };
}

int main()
{
    testSwitch(3);
    ns::TestCtorDtor::TestClass ts;
    a::b::c::d d;
    d.test();
    Inherit i;
    i.test();
    return 0;
}
