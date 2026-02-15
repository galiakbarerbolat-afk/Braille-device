import React, { useState } from 'react';
import { Copy, Sparkles, Download, Loader2, Zap, Trash2 } from 'lucide-react';

export default function BrailleConverter() {
  const [inputText, setInputText] = useState('');
  const [brailleOutput, setBrailleOutput] = useState('⠺⠁⠊⠞⠊⠝⠛ ⠋⠕⠗ ⠊⠝⠏⠥⠞⠲⠲⠲');
  const [isEnhancing, setIsEnhancing] = useState(false);
  const [isConverting, setIsConverting] = useState(false);

  // Braille character mapping
  const brailleMap = {
    'a': '⠁', 'b': '⠃', 'c': '⠉', 'd': '⠙', 'e': '⠑',
    'f': '⠋', 'g': '⠛', 'h': '⠓', 'i': '⠊', 'j': '⠚',
    'k': '⠅', 'l': '⠇', 'm': '⠍', 'n': '⠝', 'o': '⠕',
    'p': '⠏', 'q': '⠟', 'r': '⠗', 's': '⠎', 't': '⠞',
    'u': '⠥', 'v': '⠧', 'w': '⠺', 'x': '⠭', 'y': '⠽', 'z': '⠵',
    '1': '⠼⠁', '2': '⠼⠃', '3': '⠼⠉', '4': '⠼⠙', '5': '⠼⠑',
    '6': '⠼⠋', '7': '⠼⠛', '8': '⠼⠓', '9': '⠼⠊', '0': '⠼⠚',
    ' ': ' ', '.': '⠲', ',': '⠂', '?': '⠦', '!': '⠖',
    ';': '⠆', ':': '⠒', '-': '⠤', '\'': '⠄', '"': '⠦',
    '(': '⠶', ')': '⠶', '\n': '\n',
  };

  const capitalIndicator = '⠠';

  // Convert text to Braille
  const textToBraille = (text) => {
    if (!text) return '';
    
    let braille = '';
    for (let char of text) {
      const lowerChar = char.toLowerCase();
      
      if (char !== lowerChar && brailleMap[lowerChar]) {
        braille += capitalIndicator + brailleMap[lowerChar];
      } else if (brailleMap[lowerChar]) {
        braille += brailleMap[lowerChar];
      } else {
        braille += '⠿';
      }
    }
    
    return braille;
  };

  // Get binary pattern for hardware
  const getBinaryPattern = (brailleText) => {
    const patterns = [];
    
    for (let char of brailleText) {
      const codePoint = char.codePointAt(0);
      
      if (codePoint >= 0x2800 && codePoint <= 0x28FF) {
        const pattern = codePoint - 0x2800;
        const dots = [];
        for (let i = 0; i < 6; i++) {
          if (pattern & (1 << i)) {
            dots.push(i + 1);
          }
        }
        
        patterns.push({
          char: char,
          decimal: pattern,
          binary: pattern.toString(2).padStart(6, '0'),
          dots: dots
        });
      }
    }
    
    return patterns;
  };

  // Convert text with animation
  const handleConvert = async () => {
    if (!inputText.trim()) {
      setBrailleOutput('⠺⠁⠊⠞⠊⠝⠛ ⠋⠕⠗ ⠊⠝⠏⠥⠞⠲⠲⠲');
      return;
    }

    setIsConverting(true);
    
    // Small delay for visual feedback
    await new Promise(resolve => setTimeout(resolve, 300));
    
    const braille = textToBraille(inputText);
    setBrailleOutput(braille);
    setIsConverting(false);
  };

  // AI Enhancement using real Claude API
  const handleAIEnhance = async () => {
    if (!inputText.trim()) {
      alert('Please enter some text first!');
      return;
    }

    setIsEnhancing(true);

    try {
      const response = await fetch("https://api.anthropic.com/v1/messages", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          model: "claude-sonnet-4-20250514",
          max_tokens: 1000,
          messages: [
            {
              role: "user",
              content: `You are helping to prepare text for Braille conversion. Please enhance the following text by:
1. Correcting any spelling errors
2. Improving grammar and punctuation
3. Making the text clearer and more concise
4. Keeping the meaning intact
5. Optimizing for tactile reading (shorter sentences, clear structure)

Return ONLY the enhanced text, no explanations or comments.

Text to enhance:
${inputText}`
            }
          ],
        })
      });

      const data = await response.json();
      
      if (data.content && data.content[0] && data.content[0].text) {
        const enhancedText = data.content[0].text.trim();
        setInputText(enhancedText);
        
        // Auto-convert after enhancement
        setTimeout(() => {
          const braille = textToBraille(enhancedText);
          setBrailleOutput(braille);
        }, 300);
      }
    } catch (error) {
      console.error('AI Enhancement Error:', error);
      alert('AI enhancement failed. Please try again.');
    } finally {
      setIsEnhancing(false);
    }
  };

  // Copy Braille to clipboard
  const handleCopy = async () => {
    if (brailleOutput === '⠺⠁⠊⠞⠊⠝⠛ ⠋⠕⠗ ⠊⠝⠏⠥⠞⠲⠲⠲') {
      alert('No Braille output to copy!');
      return;
    }

    try {
      await navigator.clipboard.writeText(brailleOutput);
      alert('✓ Copied to clipboard!');
    } catch (err) {
      alert('Failed to copy to clipboard');
    }
  };

  // Export hardware-ready data
  const handleExport = () => {
    if (!inputText.trim()) {
      alert('No data to export!');
      return;
    }

    const exportData = {
      metadata: {
        timestamp: new Date().toISOString(),
        format: 'braille-v1.0',
        encoding: 'unicode',
      },
      input: {
        text: inputText,
        charCount: inputText.length,
      },
      output: {
        braille: brailleOutput,
        unicode: Array.from(brailleOutput).map(c => c.codePointAt(0)),
        binaryPatterns: getBinaryPattern(brailleOutput),
      },
      hardware: {
        compatible: true,
        readyForIntegration: true,
        supportedDevices: ['braille-display', 'embosser', 'custom-tactile'],
      }
    };

    const blob = new Blob([JSON.stringify(exportData, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `braille-export-${Date.now()}.json`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  };

  // Clear all
  const handleClear = () => {
    setInputText('');
    setBrailleOutput('⠺⠁⠊⠞⠊⠝⠛ ⠋⠕⠗ ⠊⠝⠏⠥⠞⠲⠲⠲');
  };

  // Auto-convert on input change
  React.useEffect(() => {
    const timer = setTimeout(() => {
      if (inputText.trim()) {
        const braille = textToBraille(inputText);
        setBrailleOutput(braille);
      }
    }, 500);

    return () => clearTimeout(timer);
  }, [inputText]);

  return (
    <div className="min-h-screen bg-gradient-to-br from-purple-600 to-indigo-700 p-6">
      <div className="max-w-6xl mx-auto bg-white rounded-2xl shadow-2xl p-8">
        {/* Header */}
        <div className="text-center mb-8">
          <h1 className="text-4xl font-bold text-gray-800 mb-2">
            🔤 AI-Powered Braille Converter
          </h1>
          <p className="text-gray-600 text-lg">Real-Time Text to Braille Translation</p>
          <span className="inline-block bg-purple-600 text-white px-4 py-1 rounded-full text-sm mt-3">
            ✨ Real Claude AI Enhancement
          </span>
        </div>

        {/* Main Content Grid */}
        <div className="grid md:grid-cols-2 gap-6 mb-6">
          {/* Input Panel */}
          <div className="bg-gray-50 rounded-xl p-6 border-2 border-gray-200">
            <h2 className="text-xl font-semibold text-gray-800 mb-4 flex items-center gap-2">
              <span>📝</span> Text Input
            </h2>
            <textarea
              value={inputText}
              onChange={(e) => setInputText(e.target.value)}
              placeholder="Enter text here or use AI enhancement...

Try typing: 'Hello World' or paste any text!"
              className="w-full h-64 p-4 border-2 border-gray-300 rounded-lg text-lg resize-none focus:border-purple-500 focus:outline-none"
            />
            <div className="flex flex-wrap gap-3 mt-4">
              <button
                onClick={handleConvert}
                disabled={isConverting}
                className="flex items-center gap-2 bg-purple-600 text-white px-5 py-2.5 rounded-lg font-semibold hover:bg-purple-700 transition-all disabled:opacity-50"
              >
                {isConverting ? <Loader2 className="animate-spin" size={18} /> : <Zap size={18} />}
                Convert to Braille
              </button>
              <button
                onClick={handleAIEnhance}
                disabled={isEnhancing}
                className="flex items-center gap-2 bg-gradient-to-r from-purple-600 to-indigo-600 text-white px-5 py-2.5 rounded-lg font-semibold hover:shadow-lg transition-all disabled:opacity-50"
              >
                {isEnhancing ? <Loader2 className="animate-spin" size={18} /> : <Sparkles size={18} />}
                {isEnhancing ? 'Enhancing...' : 'AI Enhance'}
              </button>
              <button
                onClick={handleClear}
                className="flex items-center gap-2 bg-gray-600 text-white px-5 py-2.5 rounded-lg font-semibold hover:bg-gray-700 transition-all"
              >
                <Trash2 size={18} />
                Clear
              </button>
            </div>
          </div>

          {/* Output Panel */}
          <div className="bg-gray-50 rounded-xl p-6 border-2 border-gray-200">
            <h2 className="text-xl font-semibold text-gray-800 mb-4 flex items-center gap-2">
              <span>⠃</span> Braille Output
            </h2>
            <div className="w-full h-64 p-4 bg-white border-2 border-gray-300 rounded-lg text-2xl font-mono overflow-auto break-words leading-relaxed">
              {brailleOutput}
            </div>
            <div className="flex flex-wrap gap-3 mt-4">
              <button
                onClick={handleCopy}
                className="flex items-center gap-2 bg-purple-600 text-white px-5 py-2.5 rounded-lg font-semibold hover:bg-purple-700 transition-all"
              >
                <Copy size={18} />
                Copy Braille
              </button>
              <button
                onClick={handleExport}
                className="flex items-center gap-2 bg-gray-600 text-white px-5 py-2.5 rounded-lg font-semibold hover:bg-gray-700 transition-all"
              >
                <Download size={18} />
                Export (Hardware Ready)
              </button>
            </div>
          </div>
        </div>

        {/* Status Bar */}
        <div className="bg-gray-50 rounded-lg p-4 flex justify-between items-center text-sm">
          <span className="text-gray-600 font-semibold">
            Characters: {inputText.length}
          </span>
          <span className="text-green-600 font-semibold">
            ✓ Modular Architecture for Hardware Integration
          </span>
        </div>

        {/* Info Section */}
        <div className="mt-6 bg-blue-50 border-l-4 border-purple-600 p-6 rounded-lg">
          <h3 className="font-semibold text-gray-800 mb-3">🚀 Features & Hardware Integration</h3>
          <ul className="space-y-2 text-gray-700">
            <li>
              <strong>✨ Real AI Enhancement:</strong> Powered by Claude AI - corrects spelling, 
              improves grammar, and optimizes text for Braille conversion
            </li>
            <li>
              <strong>⚡ Real-time Conversion:</strong> Instant text-to-Braille translation 
              using Unicode Braille patterns
            </li>
            <li>
              <strong>🔧 Hardware Ready:</strong> Modular architecture for easy integration with 
              scanners, OCR, physical Braille displays, and embossing devices
            </li>
            <li>
              <strong>💾 Export Function:</strong> Generates hardware-compatible JSON with binary 
              dot patterns for Arduino, Raspberry Pi, or custom devices
            </li>
            <li>
              <strong>🌐 Works Everywhere:</strong> Compatible with Safari, Chrome, Firefox, and all modern browsers
            </li>
          </ul>
        </div>
      </div>
    </div>
  );
}
