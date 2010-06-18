<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:msxml="urn:schemas-microsoft-com:xslt">
<xsl:output method="html" indent="no"/>

<xsl:template match="/">
<html>
	<head>
		<style>
			table { border-collapse: collapse; }
			tr.header {font-weight:bold; background:whitesmoke;} 
			td.assembly {background:ghostwhite; padding: 5px  30px 5px  5px; } 
			td.coverage0   {background:#E79090;text-align:right;} 
			td.coverage20  {background:#D79797;text-align:right;} 
			td.coverage40  {background:#D7A0A0;text-align:right;} 
			td.coverage60  {background:#C7A7A7;text-align:right;} 
			td.coverage80  {background:#C0B0B0;text-align:right;} 
			td.coverage100 {background:#D7D7D7;text-align:right;} 
		</style>
	</head>
	<body>
	
	<table>
		<tr class="header"><td colspan="2">Coverage by assembly</td></tr>
		<xsl:variable name="unique-asms" select="/PartCoverReport/type[not(@asm=following::type/@asm)]"/>
		<xsl:for-each select="$unique-asms">
			<xsl:variable name="current-asm" select="./@asm"/>
			<tr>
				
				<td class="assembly"><xsl:value-of select="$current-asm"/></td>
				
				<xsl:variable name="codeSize" select="sum(/PartCoverReport/type[@asm=$current-asm]/method/code/pt/@len)+0"/>
				<xsl:variable name="coveredCodeSize" select="sum(/PartCoverReport/type[@asm=$current-asm]/method/code/pt[@visit>0]/@len)+0"/>
				
				<xsl:if test="$codeSize=0">
					<td class="coverage0">0%</td>
				</xsl:if>
				
				<xsl:if test="$codeSize &gt; 0">
					<xsl:variable name="coverage" select="ceiling(100 * $coveredCodeSize div $codeSize)"/>
					<xsl:if test="$coverage &gt;=  0 and $coverage &lt; 20"><td class="coverage20"><xsl:value-of select="$coverage"/>%</td></xsl:if>
					<xsl:if test="$coverage &gt;= 20 and $coverage &lt; 40"><td class="coverage40"><xsl:value-of select="$coverage"/>%</td></xsl:if>
					<xsl:if test="$coverage &gt;= 40 and $coverage &lt; 60"><td class="coverage60"><xsl:value-of select="$coverage"/>%</td></xsl:if>
					<xsl:if test="$coverage &gt;= 60 and $coverage &lt; 80"><td class="coverage80"><xsl:value-of select="$coverage"/>%</td></xsl:if>
					<xsl:if test="$coverage &gt;= 80"><td class="coverage100"><xsl:value-of select="$coverage"/>%</td></xsl:if>
				</xsl:if>

			</tr>
		</xsl:for-each>
	</table>
	</body>
</html>
</xsl:template>

</xsl:stylesheet>
