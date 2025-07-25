<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!--
  Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
-->

<xsl:output method="text"/>
<xsl:strip-space elements="*"/>
<xsl:preserve-space elements="para type"/>


<xsl:variable name="newline">
<xsl:text>
</xsl:text>
</xsl:variable>


<xsl:variable name="all-compounddefs" select="/doxygen//compounddef"/>
<xsl:variable name="all-memberdefs" select="/doxygen//memberdef"/>


<!--
  Loop over all top-level documentation elements (i.e. classes, functions,
  variables and typedefs at namespace scope). The elements are sorted by name.
  Anything in a "detail" namespace or with "_handler" in the name is skipped.
-->
<xsl:template match="/doxygen">
<xsl:text>[/
 / Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
 /
 / Distributed under the Boost Software License, Version 1.0. (See accompanying
 / file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 /]

[section:reference Reference]

[xinclude quickref.xml]

[include requirements/asynchronous_operations.qbk]
[include requirements/read_write_operations.qbk]
[include requirements/synchronous_socket_operations.qbk]
[include requirements/asynchronous_socket_operations.qbk]
[include requirements/AcceptableProtocol.qbk]
[include requirements/AcceptHandler.qbk]
[include requirements/AcceptToken.qbk]
[include requirements/AsyncRandomAccessReadDevice.qbk]
[include requirements/AsyncRandomAccessWriteDevice.qbk]
[include requirements/AsyncReadStream.qbk]
[include requirements/AsyncWriteStream.qbk]
[include requirements/BufferedHandshakeHandler.qbk]
[include requirements/BufferedHandshakeToken.qbk]
[include requirements/CancellationHandler.qbk]
[include requirements/CancellationSlot.qbk]
[include requirements/CompletionCondition.qbk]
[include requirements/ConnectCondition.qbk]
[include requirements/ConnectHandler.qbk]
[include requirements/ConnectToken.qbk]
[include requirements/ConstBufferSequence.qbk]
[include requirements/Disposition.qbk]
[include requirements/DynamicBuffer.qbk]
[include requirements/DynamicBuffer_v1.qbk]
[include requirements/DynamicBuffer_v2.qbk]
[include requirements/Endpoint.qbk]
[include requirements/EndpointSequence.qbk]
[include requirements/ExecutionContext.qbk]
[include requirements/Executor.qbk]
[include requirements/GettableSerialPortOption.qbk]
[include requirements/GettableSocketOption.qbk]
[include requirements/Handler.qbk]
[include requirements/HandshakeHandler.qbk]
[include requirements/HandshakeToken.qbk]
[include requirements/InternetProtocol.qbk]
[include requirements/IoControlCommand.qbk]
[include requirements/IoObjectService.qbk]
[include requirements/IteratorConnectHandler.qbk]
[include requirements/IteratorConnectToken.qbk]
[include requirements/MoveAcceptHandler.qbk]
[include requirements/MoveAcceptToken.qbk]
[include requirements/MutableBufferSequence.qbk]
[include requirements/NullaryToken.qbk]
[include requirements/ProtoAllocator.qbk]
[include requirements/Protocol.qbk]
[include requirements/RangeConnectHandler.qbk]
[include requirements/RangeConnectToken.qbk]
[include requirements/ReadHandler.qbk]
[include requirements/ReadToken.qbk]
[include requirements/ResolveHandler.qbk]
[include requirements/ResolveToken.qbk]
[include requirements/Service.qbk]
[include requirements/SettableSerialPortOption.qbk]
[include requirements/SettableSocketOption.qbk]
[include requirements/ShutdownHandler.qbk]
[include requirements/ShutdownToken.qbk]
[include requirements/SignalHandler.qbk]
[include requirements/SignalToken.qbk]
[include requirements/SyncRandomAccessReadDevice.qbk]
[include requirements/SyncRandomAccessWriteDevice.qbk]
[include requirements/SyncReadStream.qbk]
[include requirements/SyncWriteStream.qbk]
[include requirements/TimeTraits.qbk]
[include requirements/WaitHandler.qbk]
[include requirements/WaitToken.qbk]
[include requirements/WaitTraits.qbk]
[include requirements/WriteHandler.qbk]
[include requirements/WriteToken.qbk]

</xsl:text>

  <xsl:for-each select="
      compounddef[@kind = 'class' or @kind = 'struct'] |
      compounddef[@kind = 'namespace']/sectiondef[@kind = 'func' or @kind = 'var' or @kind = 'enum' or @kind = 'typedef']/memberdef">
    <xsl:sort select="concat((. | ancestor::*)/compoundname, '::', name, ':x')"/>
    <xsl:sort select="name"/>
    <xsl:choose>
      <xsl:when test="@kind='class' or @kind='struct'">
        <xsl:if test="
            contains(compoundname, 'asio::') and
            not(contains(compoundname, '::detail')) and
            not(contains(compoundname, '::service::key')) and
            not(contains(compoundname, '::cancellation_state::impl')) and
            not(contains(compoundname, '_handler')) and
            not(contains(compoundname, '_helper')) and
            not(contains(compoundname, 'std_allocator_void')) and
            not(contains(compoundname, 'thread_function')) and
            not(contains(compoundname, 'context_impl')) and
            not(contains(compoundname, 'initiate_')) and
            not(contains(compoundname, '_adapter')) and
            not(contains(compoundname, '_is_deprecated')) or
            contains(compoundname, 'asio::any_completion_handler')">
          <xsl:call-template name="class"/>
        </xsl:if>
      </xsl:when>
      <xsl:otherwise>
        <xsl:if test="
            not(contains(ancestor::*/compoundname, '::detail')) and
            not(contains(ancestor::*/compoundname, '::service::key')) and
            not(contains(ancestor::*/compoundname, '::cancellation_state::impl')) and
            not(contains(ancestor::*/compoundname, '_helper')) and
            not(contains(ancestor::*/compoundname, 'initiate_')) and
            not(contains(name, '_helper')) and
            not(contains(name, 'std_allocator_void')) and
            not(contains(name, 'thread_function')) and
            not(contains(name, 'io_context_impl')) and
            not(contains(name, 'initiate_')) and
            not(contains(name, '_adapter')) and
            not(contains(name, '_is_deprecated'))">
          <xsl:call-template name="namespace-memberdef"/>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>

  <xsl:value-of select="$newline"/>
  <xsl:text>[endsect]</xsl:text>
  <xsl:value-of select="$newline"/>

</xsl:template>


<!--========== Utilities ==========-->

<xsl:template name="strip-asio-ns">
  <xsl:param name="name"/>
  <xsl:choose>
    <xsl:when test="contains($name, 'boost::system::is_error_code_enum')">
      <xsl:value-of select="$name"/>
    </xsl:when>
    <xsl:when test="contains($name, 'asio::')">
      <xsl:value-of select="substring-after($name, 'asio::')"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$name"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="strip-ns">
  <xsl:param name="name"/>
  <xsl:choose>
    <xsl:when test="contains($name, 'boost::system::is_error_code_enum')">
      <xsl:value-of select="$name"/>
    </xsl:when>
    <xsl:when test="contains($name, '::') and contains($name, '&lt;')">
      <xsl:choose>
        <xsl:when test="string-length(substring-before($name, '::')) &lt; string-length(substring-before($name, '&lt;'))">
          <xsl:call-template name="strip-ns">
            <xsl:with-param name="name" select="substring-after($name, '::')"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$name"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="contains($name, '::')">
      <xsl:call-template name="strip-ns">
        <xsl:with-param name="name" select="substring-after($name, '::')"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$name"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="cleanup-type">
  <xsl:param name="name"/>
  <xsl:param name="function-name"/>
  <xsl:variable name="type">
    <xsl:choose>
      <xsl:when test="contains($name, 'BOOST_ASIO_DECL ')">
        <xsl:value-of select="substring-after($name, 'BOOST_ASIO_DECL ')"/>
      </xsl:when>
      <xsl:when test="contains($name, 'BOOST_ASIO_DECL')">
        <xsl:value-of select="substring-after($name, 'BOOST_ASIO_DECL')"/>
      </xsl:when>
      <xsl:when test="$name = 'virtual'"></xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$name"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="$type='void_or_deduced'">
      <xsl:text>``[link boost_asio.reference.asynchronous_operations.automatic_deduction_of_initiating_function_return_type ['DEDUCED]]``</xsl:text>
    </xsl:when>
    <xsl:when test="$type='auto' and starts-with($function-name, 'async_')">
      <xsl:text>``[link boost_asio.reference.asynchronous_operations.automatic_deduction_of_initiating_function_return_type ['DEDUCED]]``</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$type"/>
    </xsl:otherwise>   
  </xsl:choose>
</xsl:template>


<xsl:template name="make-id">
  <xsl:param name="name"/>
  <xsl:param name="static"/>
  <xsl:choose>
    <xsl:when test="$name='query' and $static='yes'">
      <xsl:text>query__static</xsl:text>
    </xsl:when>
    <xsl:when test="contains($name, 'boost::system::')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="substring-after($name, 'boost::system::')"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, 'boost::asio::error::')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, 'boost::asio::error::'), substring-after($name, 'boost::asio::error::'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '::')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '::'), '__', substring-after($name, '::'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '=')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '='), '_eq_', substring-after($name, '='))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '!')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '!'), '_not_', substring-after($name, '!'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '-&gt;')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '-&gt;'), '_arrow_', substring-after($name, '-&gt;'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '&lt;')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '&lt;'), '_lt_', substring-after($name, '&lt;'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '&gt;')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '&gt;'), '_gt_', substring-after($name, '&gt;'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '&amp;')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '&amp;'), '_amp_', substring-after($name, '&amp;'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '[')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '['), '_lb_', substring-after($name, '['))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, ']')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, ']'), '_rb_', substring-after($name, ']'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '(')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '('), '_lp_', substring-after($name, '('))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, ')')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, ')'), '_rp_', substring-after($name, ')'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '+')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '+'), '_plus_', substring-after($name, '+'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '-')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '-'), '_minus_', substring-after($name, '-'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '*')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '*'), '_star_', substring-after($name, '*'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '~')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '~'), '_', substring-after($name, '~'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '^')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '^'), '_hat_', substring-after($name, '^'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '|')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '|'), '_pipe_', substring-after($name, '|'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, ',')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, ','), '_comma_', substring-after($name, ','))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '&quot;')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '&quot;'), '_quot_', substring-after($name, '&quot;'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, '...')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, '...'), '_ellipsis_', substring-after($name, '...'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, ' ')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, ' '), '_', substring-after($name, ' '))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($name, 'boost__posix_time__ptime')">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name"
         select="concat(substring-before($name, 'boost__posix_time__ptime'), 'ptime', substring-after($name, 'boost__posix_time__ptime'))"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$name"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--========== Markup ==========-->

<xsl:template match="para" mode="markup">
<xsl:text>
</xsl:text><xsl:apply-templates mode="markup"/><xsl:text>
</xsl:text>
</xsl:template>


<xsl:template match="para" mode="markup-nested">
<xsl:apply-templates mode="markup-nested"/>
</xsl:template>


<xsl:template match="title" mode="markup">
  <xsl:variable name="title">
    <xsl:value-of select="."/>
  </xsl:variable>
  <xsl:if test="string-length($title) > 0">
[heading <xsl:value-of select="."/>]
  </xsl:if>
</xsl:template>


<xsl:template match="programlisting" mode="markup">
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="$newline"/>
  <xsl:apply-templates mode="codeline"/>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="$newline"/>
</xsl:template>


<xsl:template match="programlisting" mode="markup-nested">
  <xsl:value-of select="$newline"/>
  <xsl:text>``</xsl:text>
  <xsl:value-of select="$newline"/>
  <xsl:apply-templates mode="codeline"/>
  <xsl:if test="substring(., string-length(.)) = $newline">
    <xsl:value-of select="$newline"/>
  </xsl:if>
  <xsl:text>``</xsl:text>
  <xsl:value-of select="$newline"/>
</xsl:template>


<xsl:template match="codeline" mode="codeline">
  <xsl:variable name="code">
    <xsl:apply-templates mode="codeline"/>
  </xsl:variable>
  <xsl:if test="string-length(.) &gt; 0">
    <xsl:choose>
      <xsl:when test="position() = 1 and starts-with($code, ' ')">
        <xsl:text> </xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>  </xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:value-of select="$code"/>
  <xsl:value-of select="$newline"/>
</xsl:template>


<xsl:template match="sp" mode="markup">
<xsl:text> </xsl:text>
</xsl:template>


<xsl:template match="sp" mode="markup-nested">
<xsl:text> </xsl:text>
</xsl:template>


<xsl:template match="sp" mode="codeline">
  <xsl:text> </xsl:text>
</xsl:template>


<xsl:template match="linebreak" mode="markup">
<xsl:text>

</xsl:text>
</xsl:template>


<xsl:template match="linebreak" mode="markup-nested">
<xsl:text>

</xsl:text>
</xsl:template>


<xsl:template match="computeroutput" mode="markup">
<xsl:text>`</xsl:text><xsl:value-of select="."/><xsl:text>`</xsl:text>
</xsl:template>


<xsl:template match="computeroutput" mode="markup-nested">
<xsl:text>`</xsl:text><xsl:value-of select="."/><xsl:text>`</xsl:text>
</xsl:template>


<xsl:template match="listitem" mode="markup">
* <xsl:call-template name="strip-leading-whitespace">
  <xsl:with-param name="text">
    <xsl:apply-templates mode="markup"/>
  </xsl:with-param>
</xsl:call-template>
</xsl:template>


<xsl:template match="bold" mode="markup">[*<xsl:apply-templates mode="markup"/>]</xsl:template>


<xsl:template match="emphasis" mode="markup">['<xsl:apply-templates mode="markup"/>]</xsl:template>


<xsl:template match="parameterlist" mode="markup">
  <xsl:choose>
    <xsl:when test="@kind='param'">
[heading Parameters]
    </xsl:when>
    <xsl:when test="@kind='exception'">
[heading Exceptions]
    </xsl:when>
  </xsl:choose>

[variablelist
  <xsl:apply-templates mode="markup"/>
]
</xsl:template>


<xsl:template match="parameteritem" mode="markup">
[[<xsl:value-of select="parameternamelist"/>][<xsl:apply-templates select="parameterdescription" mode="markup-nested"/>]]
</xsl:template>


<xsl:template match="simplesect" mode="markup">
  <xsl:choose>
    <xsl:when test="@kind='return'">
[heading Return Value]
      <xsl:apply-templates mode="markup"/>
    </xsl:when>
    <xsl:when test="@kind='see'">
    </xsl:when>
    <xsl:when test="@kind='note'">
[heading Remarks]
      <xsl:apply-templates mode="markup"/>
    </xsl:when>
    <xsl:when test="@kind='attention'">
[heading Attention]
      <xsl:apply-templates mode="markup"/>
    </xsl:when>
    <xsl:when test="@kind='par'">
      <xsl:if test="not(starts-with(title, 'Concepts:'))">
        <xsl:apply-templates mode="markup"/>
      </xsl:if>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates mode="markup"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="text()" mode="markup">
  <xsl:variable name="text" select="."/>
  <xsl:variable name="starts-with-whitespace" select="
      starts-with($text, ' ') or starts-with($text, $newline)"/>
  <xsl:variable name="preceding-node-name">
    <xsl:for-each select="preceding-sibling::*">
      <xsl:if test="position() = last()">
        <xsl:value-of select="local-name()"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:variable name="after-newline" select="
      $preceding-node-name = 'programlisting' or
      $preceding-node-name = 'linebreak'"/>
  <xsl:choose>
    <xsl:when test="$starts-with-whitespace and $after-newline">
      <xsl:call-template name="escape-text">
        <xsl:with-param name="text">
          <xsl:call-template name="strip-leading-whitespace">
            <xsl:with-param name="text" select="$text"/>
          </xsl:call-template>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="escape-text">
        <xsl:with-param name="text" select="$text"/>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="text()" mode="markup-nested">
  <xsl:variable name="text" select="."/>
  <xsl:variable name="starts-with-whitespace" select="
      starts-with($text, ' ') or starts-with($text, $newline)"/>
  <xsl:variable name="preceding-node-name">
    <xsl:for-each select="preceding-sibling::*">
      <xsl:if test="position() = last()">
        <xsl:value-of select="local-name()"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:variable name="after-newline" select="
      $preceding-node-name = 'programlisting' or
      $preceding-node-name = 'linebreak'"/>
  <xsl:choose>
    <xsl:when test="$starts-with-whitespace and $after-newline">
      <xsl:call-template name="escape-text">
        <xsl:with-param name="text">
          <xsl:call-template name="strip-leading-whitespace">
            <xsl:with-param name="text" select="$text"/>
          </xsl:call-template>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="escape-text">
        <xsl:with-param name="text" select="$text"/>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="strip-leading-whitespace">
  <xsl:param name="text"/>
  <xsl:choose>
    <xsl:when test="starts-with($text, ' ')">
      <xsl:call-template name="strip-leading-whitespace">
        <xsl:with-param name="text" select="substring($text, 2)"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="starts-with($text, $newline)">
      <xsl:call-template name="strip-leading-whitespace">
        <xsl:with-param name="text" select="substring($text, 2)"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$text"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="escape-text">
  <xsl:param name="text"/>
  <xsl:choose>
    <xsl:when test="contains($text, '_')">
      <xsl:value-of select="substring-before($text, '_')"/>
      <xsl:text>\_</xsl:text>
      <xsl:call-template name="escape-text">
        <xsl:with-param name="text" select="substring-after($text, '_')"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$text"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="escape-name">
  <xsl:param name="text"/>
  <xsl:choose>
    <xsl:when test="contains($text, '[')">
      <xsl:value-of select="substring-before($text, '[')"/>
      <xsl:text>\[</xsl:text>
      <xsl:call-template name="escape-name">
        <xsl:with-param name="text" select="substring-after($text, '[')"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($text, ']')">
      <xsl:value-of select="substring-before($text, ']')"/>
      <xsl:text>\]</xsl:text>
      <xsl:call-template name="escape-name">
        <xsl:with-param name="text" select="substring-after($text, ']')"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="contains($text, '...')">
      <xsl:value-of select="substring-before($text, '...')"/>
      <xsl:text>\.\.\.</xsl:text>
      <xsl:call-template name="escape-name">
        <xsl:with-param name="text" select="substring-after($text, '...')"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$text"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="ref[@kindref='compound']" mode="markup">
  <xsl:variable name="name">
    <xsl:value-of select="."/>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="contains(@refid, 'asio') or contains($name, 'asio::')">
      <xsl:variable name="dox-ref-id" select="@refid"/>
      <xsl:variable name="ref-name">
        <xsl:call-template name="strip-asio-ns">
          <xsl:with-param name="name"
            select="(($all-compounddefs)[@id=$dox-ref-id])[1]/compoundname"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:variable name="ref-id">
        <xsl:call-template name="make-id">
          <xsl:with-param name="name" select="$ref-name"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:text>[link boost_asio.reference.</xsl:text>
      <xsl:value-of select="$ref-id"/>
      <xsl:text> `</xsl:text>
      <xsl:value-of name="text" select="$ref-name"/>
      <xsl:text>`]</xsl:text>
    </xsl:when>
    <xsl:when test="@refid = 'asynchronous_operation'">
      <xsl:text>[link boost_asio.overview.model.async_ops </xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>]</xsl:text>
    </xsl:when>
    <xsl:when test="@refid = 'completion_token'">
      <xsl:text>[link boost_asio.overview.model.completion_tokens </xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>]</xsl:text>
    </xsl:when>
    <xsl:when test="@refid = 'async_result_requirements'">
      <xsl:text>[link boost_asio.reference.asynchronous_operations.completion_tokens_and_handlers </xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>]</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>`</xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>`</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="ref[@kindref='compound']" mode="markup-nested">
  <xsl:variable name="name">
    <xsl:value-of select="."/>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="contains(@refid, 'asio') or contains($name, 'asio::')">
      <xsl:variable name="dox-ref-id" select="@refid"/>
      <xsl:variable name="ref-name">
        <xsl:call-template name="strip-asio-ns">
          <xsl:with-param name="name"
            select="(($all-compounddefs)[@id=$dox-ref-id])[1]/compoundname"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:variable name="ref-id">
        <xsl:call-template name="make-id">
          <xsl:with-param name="name" select="$ref-name"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:text>[link boost_asio.reference.</xsl:text>
      <xsl:value-of select="$ref-id"/>
      <xsl:text> `</xsl:text>
      <xsl:value-of name="text" select="$ref-name"/>
      <xsl:text>`]</xsl:text>
    </xsl:when>
    <xsl:when test="@refid = 'asynchronous_operation'">
      <xsl:text>[link boost_asio.overview.model.async_ops </xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>]</xsl:text>
    </xsl:when>
    <xsl:when test="@refid = 'completion_token'">
      <xsl:text>[link boost_asio.overview.model.completion_tokens </xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>]</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>`</xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>`</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="ref[@kindref='member']" mode="markup">
  <xsl:variable name="dox-ref-id" select="@refid"/>
  <xsl:variable name="memberdefs" select="($all-memberdefs)[@id=$dox-ref-id]"/>
  <xsl:choose>
    <xsl:when test="contains(@refid, 'namespaceboost_1_1asio') and count($memberdefs) &gt; 0">
      <xsl:variable name="dox-compound-name" select="($memberdefs)[1]/../../compoundname"/>
      <xsl:variable name="dox-name" select="($memberdefs)[1]/name"/>
      <xsl:variable name="ref-name">
        <xsl:call-template name="strip-asio-ns">
          <xsl:with-param name="name" select="concat($dox-compound-name,'::',$dox-name)"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:variable name="ref-id">
        <xsl:call-template name="make-id">
          <xsl:with-param name="name" select="$ref-name"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:text>[link boost_asio.reference.</xsl:text>
      <xsl:value-of select="$ref-id"/>
      <xsl:text> `</xsl:text>
      <xsl:value-of name="text" select="$ref-name"/>
      <xsl:text>`]</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>`</xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>`</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="ref[@kindref='member']" mode="markup-nested">
  <xsl:variable name="dox-ref-id" select="@refid"/>
  <xsl:variable name="memberdefs" select="($all-memberdefs)[@id=$dox-ref-id]"/>
  <xsl:choose>
    <xsl:when test="contains(@refid, 'namespaceboost_1_1asio') and count($memberdefs) &gt; 0">
      <xsl:variable name="dox-compound-name" select="($memberdefs)[1]/../../compoundname"/>
      <xsl:variable name="dox-name" select="($memberdefs)[1]/name"/>
      <xsl:variable name="ref-name">
        <xsl:call-template name="strip-asio-ns">
          <xsl:with-param name="name" select="concat($dox-compound-name,'::',$dox-name)"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:variable name="ref-id">
        <xsl:call-template name="make-id">
          <xsl:with-param name="name" select="$ref-name"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:text>[link boost_asio.reference.</xsl:text>
      <xsl:value-of select="$ref-id"/>
      <xsl:text> `</xsl:text>
      <xsl:value-of name="text" select="$ref-name"/>
      <xsl:text>`]</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>`</xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>`</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="header-requirements">
  <xsl:param name="file"/>
  <xsl:value-of select="$newline"/>
  <xsl:text>[heading Requirements]</xsl:text>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="$newline"/>
  <xsl:text>['Header: ]</xsl:text>
  <xsl:text>[^boost/asio/</xsl:text>
  <xsl:value-of select="substring-after($file, 'boost/asio/')"/>
  <xsl:text>]</xsl:text>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="$newline"/>
  <xsl:text>['Convenience header: ]</xsl:text>
  <xsl:choose>
    <xsl:when test="contains($file, 'boost/asio/execution')">
      <xsl:text>[^boost/asio/execution.hpp]</xsl:text>
    </xsl:when>
    <xsl:when test="contains($file, 'boost/asio/ssl')">
      <xsl:text>[^boost/asio/ssl.hpp]</xsl:text>
    </xsl:when>
    <xsl:when test="contains($file, 'boost/asio/spawn')">
      <xsl:text>None</xsl:text>
    </xsl:when>
    <xsl:when test="contains($file, 'boost/asio/basic_deadline_timer')">
      <xsl:text>None</xsl:text>
    </xsl:when>
    <xsl:when test="contains($file, 'boost/asio/deadline_timer')">
      <xsl:text>None</xsl:text>
    </xsl:when>
    <xsl:when test="contains($file, 'boost/asio/time_traits')">
      <xsl:text>None</xsl:text>
    </xsl:when>
    <xsl:when test="contains($file, 'boost/asio/experimental')">
      <xsl:text>None</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>[^boost/asio.hpp]</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:value-of select="$newline"/>
</xsl:template>


<!--========== Class ==========-->

<xsl:template name="class">
  <xsl:variable name="class-name">
    <xsl:call-template name="strip-asio-ns">
      <xsl:with-param name="name" select="compoundname"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="escaped-class-name">
    <xsl:call-template name="escape-name">
      <xsl:with-param name="text" select="$class-name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="unqualified-class-name">
    <xsl:call-template name="strip-ns">
      <xsl:with-param name="name" select="compoundname"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="class-id">
    <xsl:call-template name="make-id">
      <xsl:with-param name="name" select="$class-name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="class-file" select="location/@file"/>
[section:<xsl:value-of select="$class-id"/><xsl:text> </xsl:text><xsl:value-of select="$class-name"/>]

[indexterm1 boost_asio.indexterm.<xsl:value-of select="$class-id"/>..<xsl:value-of select="$escaped-class-name"/>]

<xsl:apply-templates select="briefdescription" mode="markup"/><xsl:text>

</xsl:text>

<xsl:apply-templates select="templateparamlist"
 mode="class-detail"/><xsl:text>  </xsl:text><xsl:value-of select="@kind"/><xsl:text> </xsl:text><xsl:value-of
 select="$unqualified-class-name"/><xsl:if test="count(basecompoundref[not(contains(.,'::detail'))]) > 0"> :</xsl:if><xsl:text>
</xsl:text><xsl:for-each select="basecompoundref[not(contains(.,'::detail'))]">
<xsl:text>    </xsl:text><xsl:if test="@prot='public'">public </xsl:if><xsl:call-template
 name="strip-asio-ns"><xsl:with-param name="name" select="."/></xsl:call-template><xsl:if
 test="not(position() = last())">,</xsl:if><xsl:text>
</xsl:text></xsl:for-each><xsl:text>
</xsl:text>

<xsl:call-template name="class-tables">
  <xsl:with-param name="class-name" select="$class-name"/>
  <xsl:with-param name="class-id" select="$class-id"/>
  <xsl:with-param name="unqualified-class-name" select="$unqualified-class-name"/>
</xsl:call-template>

<xsl:apply-templates select="detaileddescription" mode="markup"/>

<xsl:call-template name="header-requirements">
  <xsl:with-param name="file" select="$class-file"/>
</xsl:call-template>

<xsl:call-template name="class-members">
  <xsl:with-param name="class-name" select="$class-name"/>
  <xsl:with-param name="class-id" select="$class-id"/>
  <xsl:with-param name="class-file" select="$class-file"/>
</xsl:call-template>

[endsect]
</xsl:template>


<xsl:template name="class-tables">
<xsl:param name="class-name"/>
<xsl:param name="class-id"/>
<xsl:param name="unqualified-class-name"/>
<xsl:if test="
    count(
      sectiondef[@kind='public-type'] |
      innerclass[@prot='public' and not(contains(., '_handler'))]) &gt; 0">
[heading Types]
[table
  [[Name][Description]]
<xsl:for-each select="
    sectiondef[@kind='public-type']/memberdef |
    innerclass[@prot='public' and not(contains(., '_handler')) and not(contains(., 'thread_function')) and not(contains(., 'initiate_'))]" mode="class-table">
  <xsl:sort select="concat(name, (.)[not(name)])"/>
  [
<xsl:choose>
  <xsl:when test="count(name) &gt; 0">
    [[link boost_asio.reference.<xsl:value-of select="$class-id"/>.<xsl:value-of select="name"/>
      <xsl:text> </xsl:text>[*<xsl:value-of select="name"/>]]]
    [<xsl:call-template name="escape-name"><xsl:with-param name="text" select="briefdescription"/></xsl:call-template>]
  </xsl:when>
  <xsl:otherwise>
    <xsl:variable name="type-name">
      <xsl:call-template name="strip-asio-ns">
        <xsl:with-param name="name" select="."/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="unqualified-type-name">
      <xsl:call-template name="strip-ns">
        <xsl:with-param name="name" select="."/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="type-id">
      <xsl:call-template name="make-id">
        <xsl:with-param name="name" select="$type-name"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="type-ref-id" select="@refid"/>
    [[link boost_asio.reference.<xsl:value-of select="$type-id"/>
      <xsl:text> </xsl:text>[*<xsl:value-of select="$unqualified-type-name"/>]]]
    [<xsl:value-of select="(($all-compounddefs)[@id=$type-ref-id])[1]/briefdescription"/>]
  </xsl:otherwise>
</xsl:choose>
  ]
</xsl:for-each>
]
</xsl:if>

<xsl:if test="count(sectiondef[@kind='public-func' or @kind='public-static-func']) > 0">
[heading Member Functions]
[table
  [[Name][Description]]
<xsl:for-each select="sectiondef[@kind='public-func' or @kind='public-static-func']/memberdef" mode="class-table">
  <xsl:sort select="name"/>
  <xsl:variable name="name">
    <xsl:value-of select="name"/>
  </xsl:variable>
  <xsl:variable name="escaped-name">
    <xsl:call-template name="escape-name">
      <xsl:with-param name="text" select="$name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="id">
    <xsl:call-template name="make-id">
      <xsl:with-param name="name" select="$name"/>
      <xsl:with-param name="static" select="@static"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="doxygen-id">
    <xsl:value-of select="@id"/>
  </xsl:variable>
  <xsl:variable name="overload-count">
    <xsl:value-of select="count(../memberdef[name = $name])"/>
  </xsl:variable>
  <xsl:variable name="overload-position">
    <xsl:for-each select="../memberdef[name = $name]">
      <xsl:if test="@id = $doxygen-id">
        <xsl:value-of select="position()"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:if test="$overload-position = 1">
  [
    [[link boost_asio.reference.<xsl:value-of select="$class-id"/>.<xsl:value-of select="$id"/>
      <xsl:text> </xsl:text>[*<xsl:value-of select="$escaped-name"/><xsl:text>]]</xsl:text>
      <xsl:if test="$name=$unqualified-class-name"> [constructor]</xsl:if>
      <xsl:if test="starts-with($name, '~')"> [destructor]</xsl:if>
      <xsl:if test="@static='yes'"> [static]</xsl:if>
      <xsl:text>]
    [</xsl:text><xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="not($overload-position = 1) and not(briefdescription = preceding-sibling::*/briefdescription)">
    <xsl:value-of select="$newline"/>
    <xsl:text>     [hr]</xsl:text>
    <xsl:value-of select="$newline"/>
    <xsl:text>     </xsl:text>
    <xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="$overload-position = $overload-count">
  <xsl:text>]
  ]
  </xsl:text>
  </xsl:if>
</xsl:for-each>
]
</xsl:if>

<xsl:if test="count(sectiondef[@kind='protected-func' or @kind='protected-static-func']) > 0">
[heading Protected Member Functions]
[table
  [[Name][Description]]
<xsl:for-each select="sectiondef[@kind='protected-func' or @kind='protected-static-func']/memberdef" mode="class-table">
  <xsl:sort select="name"/>
  <xsl:variable name="name">
    <xsl:value-of select="name"/>
  </xsl:variable>
  <xsl:variable name="id">
    <xsl:call-template name="make-id">
      <xsl:with-param name="name" select="$name"/>
      <xsl:with-param name="static" select="@static"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="doxygen-id">
    <xsl:value-of select="@id"/>
  </xsl:variable>
  <xsl:variable name="overload-count">
    <xsl:value-of select="count(../memberdef[name = $name])"/>
  </xsl:variable>
  <xsl:variable name="overload-position">
    <xsl:for-each select="../memberdef[name = $name]">
      <xsl:if test="@id = $doxygen-id">
        <xsl:value-of select="position()"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:if test="$overload-position = 1">
  [
    [[link boost_asio.reference.<xsl:value-of select="$class-id"/>.<xsl:value-of select="$id"/>
      <xsl:text> </xsl:text>[*<xsl:value-of select="$name"/><xsl:text>]]</xsl:text>
      <xsl:if test="$name=$unqualified-class-name"> [constructor]</xsl:if>
      <xsl:if test="starts-with($name, '~')"> [destructor]</xsl:if>
      <xsl:if test="@static='yes'"> [static]</xsl:if>
      <xsl:text>]
    [</xsl:text><xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="not($overload-position = 1) and not(briefdescription = preceding-sibling::*/briefdescription)">
    <xsl:value-of select="$newline"/>
    <xsl:text>     [hr]</xsl:text>
    <xsl:value-of select="$newline"/>
    <xsl:text>     </xsl:text>
    <xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="$overload-position = $overload-count">
  <xsl:text>]
  ]
  </xsl:text>
  </xsl:if>
</xsl:for-each>
]
</xsl:if>

<xsl:if test="$class-name = 'execution_context::service'">
<xsl:if test="count(sectiondef[@kind='private-func']) > 0">
[heading Private Member Functions]
[table
  [[Name][Description]]
<xsl:for-each select="sectiondef[@kind='private-func']/memberdef" mode="class-table">
  <xsl:sort select="name"/>
  <xsl:variable name="name">
    <xsl:value-of select="name"/>
  </xsl:variable>
  <xsl:variable name="id">
    <xsl:call-template name="make-id">
      <xsl:with-param name="name" select="$name"/>
      <xsl:with-param name="static" select="@static"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="doxygen-id">
    <xsl:value-of select="@id"/>
  </xsl:variable>
  <xsl:variable name="overload-count">
    <xsl:value-of select="count(../memberdef[name = $name])"/>
  </xsl:variable>
  <xsl:variable name="overload-position">
    <xsl:for-each select="../memberdef[name = $name]">
      <xsl:if test="@id = $doxygen-id">
        <xsl:value-of select="position()"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:if test="$overload-position = 1">
  [
    [[link boost_asio.reference.<xsl:value-of select="$class-id"/>.<xsl:value-of select="$id"/>
      <xsl:text> </xsl:text>[*<xsl:value-of select="$name"/><xsl:text>]]</xsl:text>
      <xsl:if test="$name=$unqualified-class-name"> [constructor]</xsl:if>
      <xsl:if test="starts-with($name, '~')"> [destructor]</xsl:if>
      <xsl:if test="@static='yes'"> [static]</xsl:if>
      <xsl:text>]
    [</xsl:text><xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="not($overload-position = 1) and not(briefdescription = preceding-sibling::*/briefdescription)">
    <xsl:value-of select="$newline"/>
    <xsl:text>     [hr]</xsl:text>
    <xsl:value-of select="$newline"/>
    <xsl:text>     </xsl:text>
    <xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="$overload-position = $overload-count">
  <xsl:text>]
  ]
  </xsl:text>
  </xsl:if>
</xsl:for-each>
]
</xsl:if>
</xsl:if>

<xsl:if test="count(sectiondef[@kind='public-attrib' or @kind='public-static-attrib']) > 0">
[heading Data Members]
[table
  [[Name][Description]]
<xsl:for-each select="sectiondef[@kind='public-attrib' or @kind='public-static-attrib']/memberdef" mode="class-table">
  <xsl:sort select="name"/>
  [
    [[link boost_asio.reference.<xsl:value-of select="$class-id"/>.<xsl:value-of select="name"/>
      <xsl:text> </xsl:text>[*<xsl:value-of select="name"/><xsl:text>]]</xsl:text>
      <xsl:if test="@static='yes'"> [static]</xsl:if>]
    [<xsl:value-of select="briefdescription"/>]
  ]
</xsl:for-each>
]
</xsl:if>

<xsl:if test="count(sectiondef[@kind='protected-attrib' or @kind='protected-static-attrib']/memberdef[not(name='impl_')]) > 0">
[heading Protected Data Members]
[table
  [[Name][Description]]
<xsl:for-each select="sectiondef[@kind='protected-attrib' or @kind='protected-static-attrib']/memberdef[not(name='impl_')]" mode="class-table">
  <xsl:sort select="name"/>
  [
    [[link boost_asio.reference.<xsl:value-of select="$class-id"/>.<xsl:value-of select="name"/>
      <xsl:text> </xsl:text>[*<xsl:value-of select="name"/><xsl:text>]]</xsl:text>
      <xsl:if test="@static='yes'"> [static]</xsl:if>]
    [<xsl:value-of select="briefdescription"/>]
  ]
</xsl:for-each>
]
</xsl:if>

<xsl:if test="count(sectiondef[@kind='friend']/memberdef[not(type = 'class') and not(contains(name, '_helper'))]) &gt; 0">
[heading Friends]
[table
  [[Name][Description]]
<xsl:for-each select="sectiondef[@kind='friend']/memberdef[not(type = 'class') and not(contains(name, '_helper'))]" mode="class-table">
  <xsl:sort select="name"/>
  <xsl:variable name="name">
    <xsl:value-of select="name"/>
  </xsl:variable>
  <xsl:variable name="id">
    <xsl:call-template name="make-id">
      <xsl:with-param name="name" select="$name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="doxygen-id">
    <xsl:value-of select="@id"/>
  </xsl:variable>
  <xsl:variable name="overload-count">
    <xsl:value-of select="count(../memberdef[name = $name])"/>
  </xsl:variable>
  <xsl:variable name="overload-position">
    <xsl:for-each select="../memberdef[name = $name]">
      <xsl:if test="@id = $doxygen-id">
        <xsl:value-of select="position()"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:if test="$overload-position = 1">
  [
    [[link boost_asio.reference.<xsl:value-of select="$class-id"/>.<xsl:value-of select="$id"/>
      <xsl:text> </xsl:text>[*<xsl:value-of select="$name"/><xsl:text>]]]
    [</xsl:text><xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="not($overload-position = 1) and not(briefdescription = preceding-sibling::*/briefdescription)">
    <xsl:value-of select="$newline"/>
    <xsl:text>     [hr]</xsl:text>
    <xsl:value-of select="$newline"/>
    <xsl:text>     </xsl:text>
    <xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="$overload-position = $overload-count">
  <xsl:text>]
  ]
  </xsl:text>
  </xsl:if>
</xsl:for-each>
]
</xsl:if>

<xsl:if test="count(sectiondef[@kind='related']/memberdef) &gt; 0">
[heading Related Functions]
[table
  [[Name][Description]]
<xsl:for-each select="sectiondef[@kind='related']/memberdef" mode="class-table">
  <xsl:sort select="name"/>
  <xsl:variable name="name">
    <xsl:value-of select="name"/>
  </xsl:variable>
  <xsl:variable name="id">
    <xsl:call-template name="make-id">
      <xsl:with-param name="name" select="$name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="doxygen-id">
    <xsl:value-of select="@id"/>
  </xsl:variable>
  <xsl:variable name="overload-count">
    <xsl:value-of select="count(../memberdef[name = $name])"/>
  </xsl:variable>
  <xsl:variable name="overload-position">
    <xsl:for-each select="../memberdef[name = $name]">
      <xsl:if test="@id = $doxygen-id">
        <xsl:value-of select="position()"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:if test="$overload-position = 1">
  [
    [[link boost_asio.reference.<xsl:value-of select="$class-id"/>.<xsl:value-of select="$id"/>
      <xsl:text> </xsl:text>[*<xsl:value-of select="$name"/><xsl:text>]]]
    [</xsl:text><xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="not($overload-position = 1) and not(briefdescription = preceding-sibling::*/briefdescription)">
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="$newline"/>
    <xsl:text>     </xsl:text>
    <xsl:value-of select="briefdescription"/>
  </xsl:if>
  <xsl:if test="$overload-position = $overload-count">
  <xsl:text>]
  ]
  </xsl:text>
  </xsl:if>
</xsl:for-each>
]
</xsl:if>

</xsl:template>


<xsl:template name="class-members">
<xsl:param name="class-name"/>
<xsl:param name="class-id"/>
<xsl:param name="class-file"/>
<xsl:apply-templates select="sectiondef[@kind='public-type' or @kind='public-func' or @kind='public-static-func' or @kind='public-attrib' or @kind='public-static-attrib' or @kind='protected-func' or @kind='protected-static-func' or @kind='protected-attrib' or @kind='protected-static-attrib' or @kind='friend' or @kind='related']/memberdef[not(type = 'class') and not(contains(name, '_helper')) and not(name = 'impl_')]" mode="class-detail">
  <xsl:sort select="name"/>
  <xsl:with-param name="class-name" select="$class-name"/>
  <xsl:with-param name="class-id" select="$class-id"/>
  <xsl:with-param name="class-file" select="$class-file"/>
</xsl:apply-templates>
<xsl:if test="$class-name = 'execution_context::service'">
  <xsl:apply-templates select="sectiondef[@kind='private-func']/memberdef[not(type = 'class') and not(contains(name, '_helper'))]" mode="class-detail">
    <xsl:sort select="name"/>
    <xsl:with-param name="class-name" select="$class-name"/>
    <xsl:with-param name="class-id" select="$class-id"/>
    <xsl:with-param name="class-file" select="$class-file"/>
  </xsl:apply-templates>
</xsl:if>
</xsl:template>


<!-- Class detail -->

<xsl:template match="memberdef" mode="class-detail">
  <xsl:param name="class-name"/>
  <xsl:param name="class-id"/>
  <xsl:param name="class-file"/>
  <xsl:variable name="name">
    <xsl:value-of select="name"/>
  </xsl:variable>
  <xsl:variable name="escaped-name">
    <xsl:call-template name="escape-name">
      <xsl:with-param name="text" select="$name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="escaped-class-name">
    <xsl:call-template name="escape-name">
      <xsl:with-param name="text" select="$class-name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="id">
    <xsl:call-template name="make-id">
      <xsl:with-param name="name" select="$name"/>
      <xsl:with-param name="static" select="@static"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="doxygen-id">
    <xsl:value-of select="@id"/>
  </xsl:variable>
  <xsl:variable name="overload-count">
    <xsl:value-of select="count(../memberdef[name = $name])"/>
  </xsl:variable>
  <xsl:variable name="overload-position">
    <xsl:for-each select="../memberdef[name = $name]">
      <xsl:if test="@id = $doxygen-id">
        <xsl:value-of select="position()"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>

<xsl:if test="$overload-count &gt; 1 and $overload-position = 1">
[section:<xsl:value-of select="$id"/><xsl:text> </xsl:text>
<xsl:value-of select="$class-name"/>::<xsl:value-of select="$escaped-name"/>]

<xsl:text>[indexterm2 </xsl:text>
<xsl:text>boost_asio.indexterm.</xsl:text>
<xsl:value-of select="$class-id"/>
<xsl:text>.</xsl:text>
<xsl:value-of select="$id"/>
<xsl:text>..</xsl:text>
<xsl:value-of select="$escaped-name"/>
<xsl:text>..</xsl:text>
<xsl:value-of select="$escaped-class-name"/>
<xsl:text>] </xsl:text>

<xsl:apply-templates select="briefdescription" mode="markup"/><xsl:text>
</xsl:text>

<xsl:for-each select="../memberdef[name = $name]">
<xsl:if test="position() &gt; 1 and not(briefdescription = preceding-sibling::*/briefdescription)">
  <xsl:value-of select="$newline"/>
  <xsl:apply-templates select="briefdescription" mode="markup"/>
  <xsl:value-of select="$newline"/>
</xsl:if>
<xsl:text>
</xsl:text><xsl:apply-templates select="templateparamlist" mode="class-detail"/>
<xsl:text>  </xsl:text>
 <xsl:if test="@explicit='yes'">explicit </xsl:if>
 <xsl:if test="@static='yes'">static </xsl:if>
 <xsl:if test="@virt='virtual'">virtual </xsl:if>
 <xsl:variable name="stripped-type">
  <xsl:call-template name="cleanup-type">
    <xsl:with-param name="name" select="type"/>
    <xsl:with-param name="function-name" select="name"/>
  </xsl:call-template>
 </xsl:variable>
 <xsl:if test="string-length($stripped-type) &gt; 0">
 <xsl:value-of select="$stripped-type"/><xsl:text> </xsl:text>
</xsl:if>``[link boost_asio.reference.<xsl:value-of select="$class-id"/>.<xsl:value-of
 select="$id"/>.overload<xsl:value-of select="position()"/><xsl:text> </xsl:text><xsl:value-of
 select="name"/>]``(<xsl:apply-templates select="param"
 mode="class-detail"/>)<xsl:if test="@const='yes'"> const</xsl:if>;
<xsl:text>  ``  [''''&amp;raquo;'''</xsl:text>
<xsl:text> [link boost_asio.reference.</xsl:text>
<xsl:value-of select="$class-id"/>.<xsl:value-of
 select="$id"/>.overload<xsl:value-of select="position()"/>
<xsl:text> more...]]``
</xsl:text>
</xsl:for-each>
</xsl:if>

[section:<xsl:if test="$overload-count = 1"><xsl:value-of select="$id"/></xsl:if>
<xsl:if test="$overload-count &gt; 1">overload<xsl:value-of select="$overload-position"/></xsl:if>
<xsl:text> </xsl:text><xsl:value-of select="$class-name"/>::<xsl:value-of select="$escaped-name"/>
<xsl:if test="$overload-count &gt; 1"> (<xsl:value-of
 select="$overload-position"/> of <xsl:value-of select="$overload-count"/> overloads)</xsl:if>]

<xsl:if test="not(starts-with($doxygen-id, ../../@id))">
<xsl:variable name="inherited-from" select="($all-compounddefs)[starts-with($doxygen-id, @id)]/compoundname"/>
<xsl:if test="not(contains($inherited-from, '::detail'))">
['Inherited from <xsl:call-template name="strip-asio-ns">
<xsl:with-param name="name" select="$inherited-from"/>
</xsl:call-template>.]<xsl:text>

</xsl:text></xsl:if></xsl:if>

<xsl:if test="$overload-count = 1">
  <xsl:text>[indexterm2 </xsl:text>
  <xsl:text>boost_asio.indexterm.</xsl:text>
  <xsl:value-of select="$class-id"/>
  <xsl:text>.</xsl:text>
  <xsl:value-of select="$id"/>
  <xsl:text>..</xsl:text>
  <xsl:value-of select="$escaped-name"/>
  <xsl:text>..</xsl:text>
  <xsl:value-of select="$escaped-class-name"/>
  <xsl:text>] </xsl:text>
</xsl:if>

<xsl:apply-templates select="briefdescription" mode="markup"/><xsl:text>
</xsl:text>

  <xsl:choose>
    <xsl:when test="@kind='typedef'">
      <xsl:call-template name="typedef" mode="class-detail">
        <xsl:with-param name="class-name" select="$class-name"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="@kind='variable'">
      <xsl:call-template name="variable" mode="class-detail"/>
    </xsl:when>
    <xsl:when test="@kind='enum'">
      <xsl:call-template name="enum" mode="class-detail">
        <xsl:with-param name="enum-name" select="$class-name"/>
        <xsl:with-param name="id" select="concat($class-id, '.', $id)"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="@kind='function'">
      <xsl:call-template name="function" mode="class-detail"/>
    </xsl:when>
    <xsl:when test="@kind='friend'">
      <xsl:call-template name="function" mode="class-detail"/>
    </xsl:when>
  </xsl:choose>

<xsl:text>
</xsl:text><xsl:apply-templates select="detaileddescription" mode="markup"/>

<xsl:if test="@kind='typedef' or @kind='friend'">
  <xsl:call-template name="header-requirements">
    <xsl:with-param name="file" select="$class-file"/>
  </xsl:call-template>
</xsl:if>

[endsect]

<xsl:if test="$overload-count &gt; 1 and $overload-position = $overload-count">
[endsect]
</xsl:if>
</xsl:template>


<xsl:template name="typedef">
<xsl:param name="class-name"/>
<xsl:text>
  </xsl:text>typedef <xsl:value-of select="type"/><xsl:text> </xsl:text><xsl:value-of select="name"/>;<xsl:text>

</xsl:text>
<xsl:if test="count(type/ref) &gt; 0 and not(contains(type, '*')) and not(contains(name, 'polymorphic_query_result_type'))">
  <xsl:variable name="class-refid">
    <xsl:choose>
      <xsl:when test="type='basic_address_iterator&lt; address_v4 &gt;'">
        <xsl:text>classasio_1_1ip_1_1basic__address__iterator_3_01address__v4_01_4</xsl:text>
      </xsl:when>
      <xsl:when test="type='basic_address_iterator&lt; address_v6 &gt;'">
        <xsl:text>classasio_1_1ip_1_1basic__address__iterator_3_01address__v6_01_4</xsl:text>
      </xsl:when>
      <xsl:when test="type='basic_address_range&lt; address_v4 &gt;'">
        <xsl:text>classasio_1_1ip_1_1basic__address__range_3_01address__v4_01_4</xsl:text>
      </xsl:when>
      <xsl:when test="type='basic_address_range&lt; address_v6 &gt;'">
        <xsl:text>classasio_1_1ip_1_1basic__address__range_3_01address__v6_01_4</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:for-each select="type/ref[1]">
          <xsl:value-of select="@refid"/>
        </xsl:for-each>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="name" select="name"/>
  <xsl:for-each select="($all-compounddefs)[@id=$class-refid]">
    <xsl:call-template name="class-tables">
      <xsl:with-param name="class-name">
        <xsl:value-of select="concat($class-name, '::', $name)"/>
      </xsl:with-param>
      <xsl:with-param name="class-id">
        <xsl:call-template name="make-id">
          <xsl:with-param name="name">
            <xsl:call-template name="strip-asio-ns">
              <xsl:with-param name="name" select="compoundname"/>
            </xsl:call-template>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:with-param>
      <xsl:with-param name="unqualified-class-name">
        <xsl:call-template name="strip-ns">
          <xsl:with-param name="name" select="compoundname"/>
        </xsl:call-template>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:apply-templates select="detaileddescription" mode="markup"/>
  </xsl:for-each>
</xsl:if>
</xsl:template>


<xsl:template name="variable">
<xsl:if test="contains(name, 'is_applicable_property_v')">
<xsl:text>
  template &lt;typename T&gt;</xsl:text>
</xsl:if>
<xsl:if test="contains(name, 'context_as')">
<xsl:text>
  template &lt;typename U&gt;</xsl:text>
</xsl:if>
<xsl:text>
  </xsl:text><xsl:if test="@static='yes'">static </xsl:if><xsl:value-of
 select="type"/><xsl:text> </xsl:text><xsl:value-of select="name"/>
 <xsl:if test="count(initializer) = 1"><xsl:text> </xsl:text>
 <xsl:value-of select="initializer"/></xsl:if>;
</xsl:template>


<xsl:template name="enum">
<xsl:param name="enum-name"/>
<xsl:param name="id"/>
<xsl:text>  enum </xsl:text>
<xsl:if test="name='cancellation_type'">
  <xsl:text>class </xsl:text>
</xsl:if>
<xsl:value-of select="name"/>
<xsl:if test="name='cancellation_type'">
  <xsl:text> : unsigned int</xsl:text>
</xsl:if>
<xsl:text>
</xsl:text><xsl:if test="count(enumvalue) &gt; 0">
<xsl:value-of select="$newline"/>
<xsl:for-each select="enumvalue">
  <xsl:text>[indexterm2 </xsl:text>
  <xsl:text>boost_asio.indexterm.</xsl:text>
  <xsl:value-of select="$id"/>
  <xsl:text>.</xsl:text>
  <xsl:value-of select="name"/>
  <xsl:text>..</xsl:text>
  <xsl:value-of select="name"/>
  <xsl:text>..</xsl:text>
  <xsl:value-of select="$enum-name"/>
  <xsl:text>]</xsl:text>
  <xsl:value-of select="$newline"/>
</xsl:for-each>
[heading Values]
[variablelist
<xsl:for-each select="enumvalue">
  [
    [<xsl:value-of select="name"/>]
    [<xsl:value-of select="briefdescription"/>]
  ]
</xsl:for-each>
]
</xsl:if>
</xsl:template>


<xsl:template name="function">
<xsl:text>
</xsl:text>
<xsl:variable name="doxygen-id">
  <xsl:value-of select="@id"/>
</xsl:variable>
<xsl:choose>
  <xsl:when test="count(($all-memberdefs)[@id=$doxygen-id]/templateparamlist) = 1">
    <xsl:apply-templates select="($all-memberdefs)[@id=$doxygen-id]/templateparamlist" mode="class-detail"/>
  </xsl:when>
  <xsl:otherwise>
    <xsl:apply-templates select="templateparamlist" mode="class-detail"/>
  </xsl:otherwise>
</xsl:choose>
<xsl:variable name="stripped-type">
 <xsl:call-template name="cleanup-type">
   <xsl:with-param name="name" select="type"/>
   <xsl:with-param name="function-name" select="name"/>
 </xsl:call-template>
</xsl:variable>
<xsl:text>  </xsl:text><xsl:if test="@static='yes'">static </xsl:if><xsl:if 
 test="@virt='virtual'">virtual </xsl:if><xsl:if
 test="string-length($stripped-type) &gt; 0"><xsl:value-of select="$stripped-type"/><xsl:text> </xsl:text></xsl:if>
<xsl:value-of select="name"/>(<xsl:apply-templates select="param"
 mode="class-detail"/>)<xsl:if test="@const='yes'"> const</xsl:if>;
</xsl:template>


<xsl:template match="templateparamlist" mode="class-detail">
<xsl:text>  </xsl:text>template&lt;<xsl:apply-templates select="param" mode="class-detail-template"/>&gt;
</xsl:template>


<xsl:template match="param" mode="class-detail-template">
<xsl:variable name="type">
  <xsl:choose>
    <xsl:when test="count(declname) &gt; 0">
      <xsl:value-of select="type"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="substring-before(type, ' ')"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>
<xsl:variable name="declname">
  <xsl:choose>
    <xsl:when test="count(declname) &gt; 0">
      <xsl:value-of select="declname"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="substring-after(type, ' ')"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>
<xsl:text>
      </xsl:text><xsl:value-of select="$type"/><xsl:text> </xsl:text>
      <xsl:choose>
        <xsl:when test="$declname = '_'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'A'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Allocator'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Arg'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Args'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Associator'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'AwaitableExecutor'">
          <xsl:value-of select="concat('``[link boost_asio.reference.Executor1 ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'Bits'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Blocking'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'BufferSequence'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'ByteType'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'C'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'CancellationCondition'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Chars'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Clock'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'CompletionHandler'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'CompletionToken'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'ConstBuffer'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Context_Service'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'ConvertibleToBuffer'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'DefaultCandidate'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Duration'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'DatagramSocketService1'">
          <xsl:value-of select="concat('``[link boost_asio.reference.DatagramSocketService ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'E'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Element'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'EndpointIterator'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Elem'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'ErrorEnum'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'ExecutionContext'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Executor'">
          <xsl:value-of select="concat('``[link boost_asio.reference.Executor1 ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'Executor2'">
          <xsl:value-of select="concat('``[link boost_asio.reference.Executor1 ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'Extent'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'F'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'F1'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'F2'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Filter'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Function'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'H'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Head'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'I'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'IoObjectService1'">
          <xsl:value-of select="concat('``[link boost_asio.reference.IoObjectService ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'Impl'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Implementation'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'InFilter'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'InitArgs'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Initiation'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'InnerExecutor'">
          <xsl:value-of select="concat('``[link boost_asio.reference.Executor1 ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'InnerExecutor1'">
          <xsl:value-of select="concat('``[link boost_asio.reference.Executor1 ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'IoObjectsOrExecutors'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Iterator'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Mask'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'MatchCondition'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'MutableBuffer'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'N'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'NewSignatures'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'OnFalse'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'OnTrue'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Ops'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'OtherAllocator'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'OtherCancellationSlot'">
          <xsl:value-of select="concat('``[link boost_asio.reference.CancellationSlot ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'OtherExecutor'">
          <xsl:value-of select="concat('``[link boost_asio.reference.Executor1 ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'OtherHandler'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'OtherSupportableProperties'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'OutFilter'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'OutstandingWork'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'PasswordCallback'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Period'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'PodType'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'PointerToPodType'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Properties'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Property'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Protocol1'">
          <xsl:value-of select="concat('``[link boost_asio.reference.Protocol ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'Ps'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'R'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Range'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'RawCompletionToken'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'RawSocketService1'">
          <xsl:value-of select="concat('``[link boost_asio.reference.RawSocketService ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'Relationship'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Rep'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Result'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Return'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'S'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'SeqPacketSocketService1'">
          <xsl:value-of select="concat('``[link boost_asio.reference.SeqPacketSocketService ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'Signature'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Signatures'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'SocketAcceptorService1' or $declname = 'SocketAcceptorService2'">
          <xsl:value-of select="concat('``[link boost_asio.reference.SocketAcceptorService ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'SocketService1' or $declname = 'SocketService2'">
          <xsl:value-of select="concat('``[link boost_asio.reference.SocketService ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'Span'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'StackAllocator'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Stream'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'StreamSocketService1'">
          <xsl:value-of select="concat('``[link boost_asio.reference.StreamSocketService ', $declname, ']``')"/>
        </xsl:when>
        <xsl:when test="$declname = 'SupportableProperties'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'T'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Tail'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Ts'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'T1'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'TN'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Time'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'TimeType'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Traits'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'U'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'V'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Values'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Vs'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'VerifyCallback'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Y'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = 'Yield'">
          <xsl:value-of select="$declname"/>
        </xsl:when>
        <xsl:when test="$declname = ''">
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="concat('``[link boost_asio.reference.', $declname, ' ', $declname, ']``')"/>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:if test="count(defval) > 0"> = <xsl:choose>
        <xsl:when test="defval = 'default_token'">
          <xsl:text>``[link boost_asio.reference.asynchronous_operations.default_completion_tokens ['DEFAULT]]``</xsl:text>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="defval"/></xsl:otherwise>
        </xsl:choose></xsl:if>
      <xsl:if test="not(position() = last())">,</xsl:if>
</xsl:template>


<xsl:template match="param" mode="class-detail">
<xsl:text>
      </xsl:text>
  <xsl:choose>
    <xsl:when test="string-length(array) &gt; 0">
      <xsl:value-of select="substring-before(type, '&amp;')"/>
      <xsl:text>(&amp;</xsl:text>
      <xsl:value-of select="declname"/>
      <xsl:text>)</xsl:text>
      <xsl:value-of select="array"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="type"/>
      <xsl:text> </xsl:text>
      <xsl:value-of select="declname"/>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:if test="count(defval) > 0"> = <xsl:choose>
    <xsl:when test="defval = 'default_token'">
      <xsl:text>``[link boost_asio.reference.asynchronous_operations.default_completion_tokens ['DEFAULT]]``</xsl:text>
    </xsl:when>
    <xsl:otherwise><xsl:value-of select="defval"/></xsl:otherwise>
  </xsl:choose></xsl:if>
  <xsl:if test="not(position() = last())">,</xsl:if>
</xsl:template>


<xsl:template match="*" mode="class-detail"/>


<!--========== Namespace ==========-->

<xsl:template name="namespace-memberdef">
  <xsl:variable name="name">
    <xsl:call-template name="strip-asio-ns">
      <xsl:with-param name="name" select="concat(../../compoundname, '::', name)"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="unqualified-name">
    <xsl:call-template name="strip-ns">
      <xsl:with-param name="name" select="$name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="id">
    <xsl:call-template name="make-id">
      <xsl:with-param name="name" select="$name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="doxygen-id">
    <xsl:value-of select="@id"/>
  </xsl:variable>
  <xsl:variable name="overload-count">
    <xsl:value-of select="count(../memberdef[name = $unqualified-name])"/>
  </xsl:variable>
  <xsl:variable name="overload-position">
    <xsl:for-each select="../memberdef[name = $unqualified-name]">
      <xsl:if test="@id = $doxygen-id">
        <xsl:value-of select="position()"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>

<xsl:if test="$overload-count &gt; 1 and $overload-position = 1">
[section:<xsl:value-of select="$id"/><xsl:text> </xsl:text><xsl:value-of select="$name"/>]

<xsl:text>[indexterm1 </xsl:text>
<xsl:text>boost_asio.indexterm.</xsl:text>
<xsl:value-of select="$id"/>
<xsl:text>..</xsl:text>
<xsl:value-of select="$name"/>
<xsl:text>]</xsl:text>
<xsl:value-of select="$newline"/>

<xsl:for-each select="($all-compounddefs)[@kind='group' and compoundname=$name]">
  <xsl:apply-templates select="briefdescription" mode="markup"/>
  <xsl:value-of select="$newline"/>
</xsl:for-each>

<xsl:for-each select="../memberdef[name = $unqualified-name]">
<xsl:variable name="stripped-type">
 <xsl:call-template name="cleanup-type">
   <xsl:with-param name="name" select="type"/>
   <xsl:with-param name="function-name" select="$unqualified-name"/>
 </xsl:call-template>
</xsl:variable>
<xsl:if test="position() = 1 or not(briefdescription = preceding-sibling::memberdef[1]/briefdescription)">
  <xsl:apply-templates select="briefdescription" mode="markup"/>
</xsl:if>
<xsl:text>
</xsl:text><xsl:apply-templates select="templateparamlist" mode="class-detail"/>
<xsl:text>  </xsl:text><xsl:if test="string-length($stripped-type) &gt; 0"><xsl:value-of
 select="$stripped-type"/><xsl:text> </xsl:text></xsl:if>``[link boost_asio.reference.<xsl:value-of
 select="$id"/>.overload<xsl:value-of select="position()"/><xsl:text> </xsl:text>
<xsl:value-of select="name"/>]``(<xsl:apply-templates select="param" mode="class-detail"/>);
<xsl:text>  ``  [''''&amp;raquo;'''</xsl:text>
<xsl:text> [link boost_asio.reference.</xsl:text>
<xsl:value-of select="$id"/>.overload<xsl:value-of select="position()"/>
<xsl:text> more...]]``
</xsl:text>
</xsl:for-each>

<xsl:for-each select="($all-compounddefs)[@kind='group' and compoundname=$name]">
  <xsl:apply-templates select="detaileddescription" mode="markup"/>
</xsl:for-each>

<xsl:call-template name="header-requirements">
  <xsl:with-param name="file" select="location/@file"/>
</xsl:call-template>

</xsl:if>

[section:<xsl:if test="$overload-count = 1"><xsl:value-of select="$id"/></xsl:if>
<xsl:if test="$overload-count &gt; 1">overload<xsl:value-of select="$overload-position"/>
</xsl:if><xsl:text> </xsl:text><xsl:value-of select="$name"/>
<xsl:if test="$overload-count &gt; 1"> (<xsl:value-of
 select="$overload-position"/> of <xsl:value-of select="$overload-count"/> overloads)</xsl:if>]

<xsl:if test="$overload-count = 1">
  <xsl:text>[indexterm1 </xsl:text>
  <xsl:text>boost_asio.indexterm.</xsl:text>
  <xsl:value-of select="$id"/>
  <xsl:text>..</xsl:text>
  <xsl:value-of select="$name"/>
  <xsl:text>]</xsl:text>
  <xsl:value-of select="$newline"/>
</xsl:if>

<xsl:apply-templates select="briefdescription" mode="markup"/><xsl:text>
</xsl:text>

  <xsl:choose>
    <xsl:when test="@kind='typedef'">
      <xsl:call-template name="typedef">
        <xsl:with-param name="class-name" select="$name"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="@kind='variable'">
      <xsl:call-template name="variable"/>
    </xsl:when>
    <xsl:when test="@kind='enum'">
      <xsl:call-template name="enum">
        <xsl:with-param name="enum-name" select="$name"/>
        <xsl:with-param name="id" select="$id"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="@kind='function'">
      <xsl:call-template name="function"/>
    </xsl:when>
  </xsl:choose>

<xsl:text>
</xsl:text><xsl:apply-templates select="detaileddescription" mode="markup"/>

<xsl:if test="$overload-count = 1">
  <xsl:call-template name="header-requirements">
    <xsl:with-param name="file" select="location/@file"/>
  </xsl:call-template>
</xsl:if>

[endsect]

<xsl:if test="$overload-count &gt; 1 and $overload-position = $overload-count">
[endsect]
</xsl:if>
</xsl:template>


</xsl:stylesheet>
