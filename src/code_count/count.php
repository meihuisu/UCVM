<?php
/**
 * A very simple code analysis stored in a single php file.
 *
 * Edit The variables just below to customize to your needs.
 */

// Variables. Change these to meet your needs.
$file_types = array('c','cpp','h', "make");
$skip_directories = array('.svn', 'files', "man", "conf", 'external', 'scripts');
$starting_directory = '../';

// Initialize. No need to change anything below this line.
$stats = array();
$stats['gen'] = array();
$stats['gen']['commented_lines'] = 0;
$stats['gen']['blank_lines'] = 0;
$stats['gen']['bracket_lines'] = 0;
$stats['gen']['comment_blocks'] = 0;
$stats['gen']['classes'] = 0;
$stats['gen']['functions'] = 0;
$stats['included_files'] = array();
$stats['excluded_files'] = array();
$stats['skip'] = $skip_directories;
$stats['file_types'] = '(' . implode('|', $file_types) . ')';

// Execute.
$totalLines = countLines($starting_directory, $stats);

// Print results.
echo 'Total lines: ' . $totalLines . '<br />';
echo 'Adjusted lines: ' . ($totalLines - $stats['gen']['commented_lines'] - $stats['gen']['blank_lines'] - $stats['gen']['bracket_lines']) . '<br />';
foreach($stats['gen'] as $key=>$val)
{
  echo ucfirst($key) . ": " . $val . "<br>";
}

echo "<br />";
echo 'Included Files (' . count($stats['included_files']) . '): <br />';
foreach($stats['included_files'] as $file_name)
{
  echo $file_name . '<br />';
}

echo "<br />";
echo 'Excluded Files & Directories (' . count($stats['excluded_files']) . '): <br />';
foreach($stats['excluded_files'] as $file_name)
{
  echo $file_name . '<br />';
}

function countLines($dir, &$stats)
{
  $lineCounter = 0;
  $dirHandle = opendir($dir);
  $path = realpath($dir);
  $nextLineIsComment = false;

  if($dirHandle)
  {
    while($file = readdir($dirHandle))
    {
      if(is_dir($path."/".$file) && ($file !== '.' && $file !== '..') && !in_array($file, $stats['skip']))
      {
        $lineCounter += countLines($path . '/' . $file, $stats);
      }
      elseif(in_array($file, $stats['skip']))
      {
        $stats['excluded_files'][] = $path . '/' . $file;
      }
      elseif($file !== '.' && $file !== '..')
      {

        // Check if we have a valid file
        $ext = _findExtension($file);
        if(preg_match("/" . $stats['file_types'] . "$/i", $ext))
        {
          $realFile = realpath($path)."/".$file;
          $fileArray = file($realFile);

          // Check content of file:
          for($i=0; $i<count($fileArray); $i++)
          {
            if($nextLineIsComment)
            {
              $stats['gen']['commented_lines']++;
              // Look for the end of the comment block
              if(strpos($fileArray[$i], '*/'))
              {
                $nextLineIsComment = false;
              }
            }
            else
            {

              // Look for a function
              if(strpos($fileArray[$i], 'function'))
              {
                $stats['gen']['functions']++;
              }

              // Look for a commented line
              if(strpos(trim($fileArray[$i]), '//') === 0)
              {
                $stats['gen']['commented_lines']++;
              }

              // Look for a class
              if(substr(trim($fileArray[$i]), 0, 5) == 'class')
              {
                $stats['gen']['classes']++;
              }

              // Look for a comment block
              if(strpos($fileArray[$i], '/*'))
              {
                $nextLineIsComment = true;
                $stats['gen']['commented_lines']++;
                $stats['gen']['comment_blocks']++;
              }

              //Look for a blank line
              if(trim($fileArray[$i]) == '')
              {
                $stats['gen']['blank_lines']++;
              }

              // Look for lines that have an open or close bracket and nothing else.
              if (trim($fileArray[$i]) == '{' || trim($fileArray[$i]) == '}')
              {
                $stats['gen']['bracket_lines']++;
              }
            }
          }
          $lineCounter += count($fileArray);

          // Mark as an included file.
          $stats['included_files'][] = $path . '/' . $file;
        }
        else
        {
          $stats['excluded_files'][] = $path . '/' . $file;
        }
      }
    }
  }
  else
  {
    echo 'Could not enter folder: ' . $dir;
  }

  return $lineCounter;
}

function _findExtension($filename)
{
  $filename = strtolower($filename) ;
  $exts = preg_split("[/\\.]", $filename) ;
  $n = count($exts)-1;
  $exts = $exts[$n];
  return $exts;
}
