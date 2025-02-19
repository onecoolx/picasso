/*
 * Copyright (c) 2025, Zhang Ji Peng
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "test.h"
#include "timeuse.h"

#include "psx_xml_token.h"

static bool _token_process(void* data, const psx_xml_token_t* token)
{
    // success
    if (token->type == PSX_XML_BEGIN) {
        EXPECT_TRUE(token->attrs.size >= 0);
        EXPECT_TRUE(token->end > token->start);
    } else if (token->type == PSX_XML_CONTENT || token->type == PSX_XML_END) {
        EXPECT_TRUE(token->end > token->start);
        EXPECT_EQ(token->attrs.size, 0);
    } else if (token->type == PSX_XML_ENTITY) {
        EXPECT_TRUE(token->end > token->start);
        EXPECT_EQ(token->attrs.size, 0);
    }
    return true;
}

static bool _token_process_fail(void* data, const psx_xml_token_t* token)
{
    return false;
}

TEST(PsxXmlTokenTest, TestNormalXML)
{
    static const char* xml_data = "<root><child attr=\"value\">\n</child></root>";
    psx_xml_tokenizer(xml_data, (uint32_t)strlen(xml_data), _token_process, nullptr);
}

TEST(PsxXmlTokenTest, TestEmpty)
{
    const char* empty_xml_data = "";
    bool ret = psx_xml_tokenizer(empty_xml_data, (uint32_t)strlen(empty_xml_data), _token_process, nullptr);
    EXPECT_FALSE(ret);
}

TEST(PsxXmlTokenTest, TestSingleTag)
{
    const char* single_tag_xml_data = "<root>\r</root>";
    psx_xml_tokenizer(single_tag_xml_data, (uint32_t)strlen(single_tag_xml_data), _token_process, nullptr);
}

TEST(PsxXmlTokenTest, TestNoAttributes)
{
    const char* no_attr_xml_data = "<root>hello world!</root>";
    psx_xml_tokenizer(no_attr_xml_data, (uint32_t)strlen(no_attr_xml_data), _token_process, nullptr);
}

TEST(PsxXmlTokenTest, TestHtmlFlatMode)
{
    const char* xml_data = "<root><children attr1=value /></root>";
    psx_xml_tokenizer(xml_data, (uint32_t)strlen(xml_data), _token_process, nullptr);
}

TEST(PsxXmlTokenTest, TestXMLInst)
{
    const char* xml_data = "<?xml information ?><root></root>";
    psx_xml_tokenizer(xml_data, (uint32_t)strlen(xml_data), _token_process, nullptr);
}

TEST(PsxXmlTokenTest, TestDocType)
{
    const char* xml_data = "<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN'"
                           "'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><root></root>";
    psx_xml_tokenizer(xml_data, (uint32_t)strlen(xml_data), _token_process, nullptr);
}

TEST(PsxXmlTokenTest, TestQuoteValue)
{
    const char* xml_data = "<root><children attr1=\"value1\" attr2='value2' attr3=value3/></root>";
    psx_xml_tokenizer(xml_data, (uint32_t)strlen(xml_data), _token_process, nullptr);

    const char* xml_data2 = "<root><children attr1=  attr2=\"\" attr3=''/></root>"; // no value
    psx_xml_tokenizer(xml_data2, (uint32_t)strlen(xml_data2), _token_process, nullptr);

    const char* xml_data3 = "<root><children checked/></root>"; // no attrname
    psx_xml_tokenizer(xml_data3, (uint32_t)strlen(xml_data3), _token_process, nullptr);

    const char* xml_data4 = "<root><children attr1=\" attr2='/></root>"; // bad case
    psx_xml_tokenizer(xml_data4, (uint32_t)strlen(xml_data4), _token_process, nullptr);
}

TEST(PsxXmlTokenTest, TestComment)
{
    const char* xml_data = "<root><children><!-- comment message -->contents</children></root>";
    psx_xml_tokenizer(xml_data, (uint32_t)strlen(xml_data), _token_process, nullptr);
}

TEST(PsxXmlTokenTest, TestEntity)
{
    const char* xml_data = "<root><children>start &amp; &AMP; contents</children></root>";
    psx_xml_tokenizer(xml_data, (uint32_t)strlen(xml_data), _token_process, nullptr);

    const char* xml_data2 = "<root><children>start &#xA9; &#169; </children></root>";
    psx_xml_tokenizer(xml_data2, (uint32_t)strlen(xml_data2), _token_process, nullptr);

    const char* xml_data3 = "<root><!ENTITY /><children>content</children></root>";
    psx_xml_tokenizer(xml_data3, (uint32_t)strlen(xml_data3), _token_process, nullptr);

    const char* xml_data4 = "<root><children>start &#; &# </children></root>"; // bad case
    psx_xml_tokenizer(xml_data4, (uint32_t)strlen(xml_data4), _token_process, nullptr);
}

TEST(PsxXmlTokenTest, TestFail)
{
    const char* xml_data = "<root></root>";
    psx_xml_tokenizer(xml_data, (uint32_t)strlen(xml_data), _token_process_fail, nullptr);
}
