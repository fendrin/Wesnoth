#!/usr/bin/python
# encoding: utf-8

"""
A script that autogenerates some information about campaigns for the
CampaignInformation wiki page. The script is a WIP.
"""

from __future__ import with_statement   # For python < 2.6
import os.path, sys
try:
    import argparse
except ImportError:
    print('Please install argparse by running "easy_install argparse"')
    sys.exit(1)

import wesnoth.wmlparser2 as wmlparser2


class Campaign:
    """
    A class for a specific campaign.
    """
    def __init__(self, parser):
        self.parser = parser
        self.name = self.parser.get_text_val("name")
        self.id = self.parser.get_text_val("id")
        self.description = self.parser.get_text_val("description")
        self.levels = len(self.parser.get_all(tag="difficulty"))
        self.credits_link = "http://wiki.wesnoth.org/Credits#" + self.id
        self.units_link = "http://units.wesnoth.org/trunk/mainline/en_US/%s.html" % self.id 

def wiki_output(campaign):
    """
    Takes a campaign instance and outputs information in wiki format
    """
    # Remove Espreon fancy but bug-inducing characters
    # FIXME: This is not elegant at all, find a better way to do it
    for i in (u"’", u"—", u'‘'):
        campaign.name = ''.join(campaign.name.split(i))
        campaign.description = ''.join(campaign.description.split(i))
    text = """== {0} ==
{1}

Difficulty levels : {2}
* [{3} Custom units]
* [{4} Credits]
""".format(campaign.name, campaign.description, campaign.levels,
campaign.units_link, campaign.credits_link)
    return text


if __name__ == "__main__":
    # Possible arguments
    arg_parser = argparse.ArgumentParser(description='campaign2wiki is a script\
which generates information about campaigns for the wiki.')
    arg_parser.add_argument('-d', '--data', default='data/',
        dest='data_dir', help="The location of wesnoth data directory")
    arg_parser.add_argument('-o', '--output', default='/tmp/CampaignWML',
        dest='output_path', help="The location of the output file.")
    arg_parser.add_argument('-w', '--wesnoth', default='./wesnoth',
        dest='wesnoth', help='The wesnoth executable location')
    args = arg_parser.parse_args()

    output = '{{Autogenerated}} '
    main = wmlparser2.Parser(args.wesnoth, None, None, False)
    main.parse_file('data/_main.cfg')
    for campaign in main.get_all(tag='campaign'):
        a = Campaign(campaign)
        output += wiki_output(a)

    with open(args.output_path, "w") as wiki_format:
        wiki_format.write(output)
