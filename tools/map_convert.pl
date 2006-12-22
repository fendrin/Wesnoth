#!/usr/bin/perl -w
#script to convert from single char maps to multiple character maps

sub printUsage(){
    print "map_convert.pl terrain.cfg map_file.cfg new_map_file.cfg\n";
}

if($#ARGV <1 ||$#ARGV>2){
    printUsage();
    exit;
}

$terrain_file=shift(@ARGV);
$map_file=shift(@ARGV);
if($#ARGV < 0){
    $new_map_file=$map_file;
    $backup=$map_file.".bak";
    system("cp $map_file $backup") && die "could not create backup file: $backup\n";
}else{
    $new_map_file=shift(@ARGV);
    if(-e "$new_map_file"){ die "New map file already exists: $new_map_file\n";}
}

if(! -r "$terrain_file"){ die "can not read terrain file: $terrain_file\n";}
if(! -r "$map_file"){ die "can not read map file: $map_file\n";}

$conversion{1}='1 _K';
$conversion{2}='2 _K';
$conversion{3}='3 _K';
$conversion{4}='4 _K';
$conversion{5}='5 _K';
$conversion{6}='6 _K';
$conversion{7}='7 _K';
$conversion{8}='8 _K';
$conversion{9}='9 _K';

#parse terrain_file
open(TERRAIN, "<$terrain_file");
$countdef=0;
$max_len=0;
while($line=<TERRAIN>){

    if($line=~/^\#ifdef/){
#skip ifdef'd comments
	$countdef+=1;
        while(($countdef >0) && ($line=<TERRAIN>)){
	    if($line=~/^\#ifdef/){ 
		$countdef+=1;
	    }
	    if($line=~/^\#endif/){ 
		$countdef-=1;
	    }
	}
    }
    elsif($line=~/^\#/){}
    else{
#parse [terrain] blocks
	if($line=~/^\[terrain\]/){
	    $char='';
	    $string='';
	    $in=1;
	    while(($in>0) && ($line=<TERRAIN>)){
		$line=~s/\s+//;
		if($line=~/^char/){
		    $char=$line;
		    $char=~s/char//;
		    ($dummy,$char)=split('=',$char);
		    $char=~s/\"//g;
		    $char=~s/\s+//g;
		    $char=substr($char,0,1);
		}elsif($line=~/^string/){
		    $string=$line;
		    $string=~s/^string//;
		    ($dummy,$string)=split('=',$string);
		    $string=~s/\"//g;
		    $string=~s/\s+//g;
		}elsif($line=~/\[\/terrain\]/){
		    $in=0;
		    if((length($char)>0) && (length($string)>0)){
#			print "$char ---> $string\n";
			if(length($string)>$max_len){
			    $max_len=length($string);
			}
			$conversion{$char}=$string;
		    }
	        }
	    }
	}
    }
}
close(TERRAIN);

$width=$max_len+2;
open(MAP, "<$map_file");
@mfile=();
$map_only=1;
while($line=<MAP>){
    push(@mfile,$line);
    if($line=~/map_data/){
	$map_only=0;
    }
}

@map=();
close(MAP);

while($#mfile){
    $line=shift(@mfile);
    if($map_only || $line=~/map_data/){
	$cont=1;
#read map assumes map is more than 1 line long.
	if(!$map_only){
	    ($dummy,$line)=split('"',$line);
	}
	if((defined($line)) && (length($line))){push(@map,$line)};
#	 print "$line\n";
	 while(($cont) && ($#mfile)){
	     $line=shift(@mfile);
	     if($line=~/\"/){$cont=0;} 
	     ($line,$dummy)=split('"',$line);
	     if(length($line)){push(@map,$line)};
	 }

	if(! $map_only){ 
	    $line="map_data=\"\n";
	    push(@newfile,$line);
	}
	 foreach(@map){
	     if($_=~/,/){die "map file appears to be converted already\n";}
	     $line='';
	     chomp;
	     chomp;
	     for($i=0;$i!=length($_);$i++){
		 $hex='';
		 $char=substr($_,$i,1);
#		 print "$char";
		 $format="%${width}.${max_len}s";
		 if(defined($conversion{$char})){
		     $hex=sprintf($format,$conversion{$char});
		 }else{
		     die "error, unrecognized map character: $char";
#		     $hex=sprintf($format,$char);
		 }
		 $line.=$hex;
		 if($i!=length($_)-1){$line.=','};
	     }
	     $line.="\n";
	     push(@newfile,$line);
#	     print "$line\n";
	 }
	if($map_only){
	    $line="\n";
	}else{
	    $line="\"\n";
	}
	push(@newfile,$line);
    }else{
	push(@newfile,$line);
    }
}

open(NEWMAP,">$new_map_file");
foreach(@newfile){
    print NEWMAP "$_";
}

