<?xml version="1.0" encoding="UTF-8"?>

<!--
	XSL
	
	Copyright 2015-2020 MicroEJ Corp. All rights reserved.
	Use of this source code is governed by a BSD-style license that can be found with this software.
	
	Bibliography:
		[XSLTREC] XSL Transformations (XSLT), Version 1.0, W3C Recommendation 16 November 1999
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<!-- This is an XML to XML generator -->
	<xsl:output method="xml" indent="yes" />

	<!-- Well known identity transformation : see 7.5 in [XSLTREC] -->
	<xsl:template match="node()|@*">
		<xsl:copy>
			<xsl:apply-templates select="node()|@*" />
		</xsl:copy>
	</xsl:template>

	<!--  Remove group before adding it again -->
	<xsl:template match="group[@name='%GROUP_NAME%']"/>
	
	<!--  Add group -->
	<xsl:template match="platform">
		<xsl:copy>
			<xsl:apply-templates select="node()|@*" />
			<group name="%GROUP_NAME%" />
		</xsl:copy>
	</xsl:template>
</xsl:stylesheet>