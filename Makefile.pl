use Cwd;

$repo_path = 'd:\REPO';

$reps = [   {name => 'TChart', tag_mask => '101001_CHART[\w\d]+'},
            {name => 'my_lib', tag_mask => '102001_MY_LIB[\w\d]+'},
            {name => 'my_gsl', tag_mask => '103001_MY_GSL[\w\d]+'}  ];

$refractometr_path = $repo_path.'/Refractometer';

open(LABELS,"$refractometr_path/Sources/gitTags");

@strs = <LABELS>;
foreach (@strs)
{
	chomp($_);
    foreach $rep (@$reps)
    {
        if ($_ =~ m/\A($$rep{tag_mask})\Z/)
        {
            if (exists($$rep{tag}) && !($$rep{tag} eq $1))
            {
                print "ERROR: diplicate tag found for $$rep{name} $$rep{tag} -> $1";
                close(LABELS); exit;
            }
            else
            {
                if (!($$rep{tag} eq $1))
                {
                    $$rep{tag} = $1;
                    print "Found label for $$rep{name}: $$rep{tag}\n";                            
                }
            }
        }
    }
}
close(LABELS);

foreach $rep (@$reps)
{
    if (!exists($$rep{tag}))
    {
        print "ERROR: no tag found for $$rep{name}";
        exit;
    }
}
print "Applying tags...\n";
foreach $rep (@$reps)
{
    chdir("$repo_path/$$rep{name}");
    $cmd = "git checkout $$rep{tag}";    
    print "$$rep{name}:\n";
    system($cmd); 
}
