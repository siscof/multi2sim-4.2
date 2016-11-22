/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/
#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <iostream>
#include <vector>
#include <string>

/*
 * CSVWriter: Writes CSV data with headers to an underlying ofstream
 * 	This wrapper is meant to look like an ofstream, but it captures
 * 	the names of each field and prints it out to a header before printing
 * 	the CSV data below.
 *
 * 	Note: the first finalize() will not print the values out, only the headers.
 * 	One way to fix this problem would be to use a sstringstream (or something)
 * 	to buffer out the values and flush them all in one go instead of passing them
 * 	directly to the underlying stream as is the case now.
 *
 * 	Example usage:
 *
 * 	CSVWriter sw(cout);               // send output to cout
 * 	sw.header("Bandwidth").field(0.5); // value ignored
 * 	sw.header("Latency").field(5);     // value ignored
 * 	sw.finalize();                      // flush the header
 * 	sw.header("Bandwidth").field(1.5); // field name ignored
 * 	sw.header("Latency").field(15);     // field name ignored
 * 	sw.finalize(); 							// values printed to csv line
 * 	sw.header("Bandwidth").field(2.5); // field name ignored
 * 	sw.header("Latency").field(25);     // field name ignored
 * 	sw.finalize(); 							// values printed to csv line
 *
 * 	The output of this example will be:
 *
 * 	Bandwidth,Latency
 * 	1.5,15
 * 	2.5,25
 *
 */


namespace DRAMSim {


	/* Returns a string whith the passed object converted to string and between square brackets */
	template <typename T>
	string squared(T object)
	{
		return "[" + std::to_string(object) + "]";
	}


	class CSVWriter {

	  private:
		std::ostream &output;
		std::vector<std::string> fieldNames;
		bool finalized;
		unsigned idx;

		// Disable copy constructor and assignment operator
		CSVWriter(const CSVWriter &);
		CSVWriter &operator=(const CSVWriter &);

	  public:
		// Constructor
		CSVWriter(std::ostream &_output) : output(_output), finalized(false), idx(0)
		{}


		void finalize()
		{
			if (!finalized)
			{
				for (unsigned i=0; i<fieldNames.size(); i++)
				{
					output << fieldNames[i] << ",";
				}
				output << std::endl << std::flush;
				finalized=true;
			}

			else
			{
				if (idx < fieldNames.size())
				{
					printf(" Number of fields doesn't match values (fields=%u, values=%u), check each value has a field name before it\n", idx, (unsigned)fieldNames.size());
				}
				idx=0;
				output << std::endl;
			}
		}


		bool isFinalized()
		{
			return finalized;
		}


		std::ostream &getOutputStream()
		{
			return output;
		}


		template <typename T>
		CSVWriter& header(const T &name)
		{
			if (!finalized)
				fieldNames.push_back(string(name));
			return *this;
		}


		template <typename T>
		CSVWriter& field(T value)
		{
			if (finalized)
			{
				output << value << ",";
				idx++;
			}
			return *this;
		}
	};
}

#endif // CSV_WRITER_H
